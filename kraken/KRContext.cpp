//
//  KRContext.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-12.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KREngine-common.h"

#include "KRContext.h"
#include "KRCamera.h"
#include "KRAudioManager.h"
#include "KRAudioSample.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

int KRContext::KRENGINE_MAX_SHADER_HANDLES;
int KRContext::KRENGINE_GPU_MEM_MAX;
int KRContext::KRENGINE_GPU_MEM_TARGET;
int KRContext::KRENGINE_MAX_TEXTURE_DIM;
int KRContext::KRENGINE_MIN_TEXTURE_DIM;
int KRContext::KRENGINE_PRESTREAM_DISTANCE;
int KRContext::KRENGINE_SYS_ALLOCATION_GRANULARITY;
int KRContext::KRENGINE_SYS_PAGE_SIZE;

#if TARGET_OS_IPHONE


#elif TARGET_OS_MAC

#elif defined(_WIN32) || defined(_WIN64)

#else

#error Unsupported Platform

#endif


const char *KRContext::extension_names[KRENGINE_NUM_EXTENSIONS] = {
    "GL_EXT_texture_storage"
};

KRContext::log_callback *KRContext::s_log_callback = NULL;
void *KRContext::s_log_callback_user_data = NULL;

KRContext::KRContext() : m_streamer(*this)
{
    m_streamingEnabled = false;
#ifdef __APPLE__
    mach_timebase_info(&m_timebase_info);
#endif
    
    m_bDetectedExtensions = false;
    m_current_frame = 0;
    m_last_memory_warning_frame = 0;
    m_last_fully_streamed_frame = 0;
    m_absolute_time = 0.0f;
    
    m_pBundleManager = new KRBundleManager(*this);
    m_pShaderManager = new KRShaderManager(*this);
    m_pTextureManager = new KRTextureManager(*this);
    m_pMaterialManager = new KRMaterialManager(*this, m_pTextureManager, m_pShaderManager);
    m_pMeshManager = new KRMeshManager(*this);
    m_pSceneManager = new KRSceneManager(*this);
    m_pAnimationManager = new KRAnimationManager(*this);
    m_pAnimationCurveManager = new KRAnimationCurveManager(*this);
    m_pSoundManager = new KRAudioManager(*this);
    m_pUnknownManager = new KRUnknownManager(*this);
    m_streamingEnabled = true;



#if defined(_WIN32) || defined(_WIN64)

    SYSTEM_INFO winSysInfo;
    GetSystemInfo(&winSysInfo);
    KRENGINE_SYS_ALLOCATION_GRANULARITY = winSysInfo.dwAllocationGranularity;
    KRENGINE_SYS_PAGE_SIZE = winSysInfo.dwPageSize;

#elif defined(__APPLE__)

    KRENGINE_SYS_PAGE_SIZE = getpagesize();
    KRENGINE_SYS_ALLOCATION_GRANULARITY = KRENGINE_SYS_PAGE_SIZE;

#else
#error Unsupported
#endif
    
    createDeviceContexts();
}

KRContext::~KRContext() {
    
    if(m_pSceneManager) {
        delete m_pSceneManager;
        m_pSceneManager = NULL;
    }
    
    if(m_pMeshManager) {
        delete m_pMeshManager;
        m_pMeshManager = NULL;
    }
    
    if(m_pTextureManager) {
        delete m_pTextureManager;
        m_pTextureManager = NULL;
    }
    
    if(m_pMaterialManager) {
        delete m_pMaterialManager;
        m_pMaterialManager = NULL;
    }
    
    if(m_pShaderManager) {
        delete m_pShaderManager;
        m_pShaderManager = NULL;
    }
    
    if(m_pAnimationManager) {
        delete m_pAnimationManager;
        m_pAnimationManager = NULL;
    }
    
    if(m_pAnimationCurveManager) {
        delete m_pAnimationCurveManager;
        m_pAnimationCurveManager = NULL;
    }
    
    if(m_pSoundManager) {
        delete m_pSoundManager;
        m_pSoundManager = NULL;
    }

    if(m_pUnknownManager) {
        delete m_pUnknownManager;
        m_pUnknownManager = NULL;
    }
    
    // The bundles must be destroyed last, as the other objects may be using mmap'ed data from bundles
    if(m_pBundleManager) {
        delete m_pBundleManager;
        m_pBundleManager = NULL;
    }
    
    destroyDeviceContexts();
}

void KRContext::SetLogCallback(log_callback *log_callback, void *user_data)
{
    s_log_callback = log_callback;
    s_log_callback_user_data = user_data;
}

void KRContext::Log(log_level level, const std::string message_format, ...)
{
    va_list args;
    va_start(args, message_format);
    
    if(s_log_callback) {
        const int LOG_BUFFER_SIZE = 32768;
        char log_buffer[LOG_BUFFER_SIZE];
        vsnprintf(log_buffer, LOG_BUFFER_SIZE, message_format.c_str(), args);
        s_log_callback(s_log_callback_user_data, std::string(log_buffer), level);
    } else {
        FILE *out_file = level == LOG_LEVEL_INFORMATION ? stdout : stderr;
        fprintf(out_file, "Kraken - INFO: ");
        vfprintf(out_file, message_format.c_str(), args);
        fprintf(out_file, "\n");
    }
    
    va_end(args);
}

KRBundleManager *KRContext::getBundleManager() {
    return m_pBundleManager;
}
KRSceneManager *KRContext::getSceneManager() {
    return m_pSceneManager;
}
KRTextureManager *KRContext::getTextureManager() {
    return m_pTextureManager;
}
KRMaterialManager *KRContext::getMaterialManager() {
    return m_pMaterialManager;
}
KRShaderManager *KRContext::getShaderManager() {
    return m_pShaderManager;
}
KRMeshManager *KRContext::getMeshManager() {
    return m_pMeshManager;
}
KRAnimationManager *KRContext::getAnimationManager() {
    return m_pAnimationManager;
}
KRAnimationCurveManager *KRContext::getAnimationCurveManager() {
    return m_pAnimationCurveManager;
}
KRAudioManager *KRContext::getAudioManager() {
    return m_pSoundManager;
}
KRUnknownManager *KRContext::getUnknownManager() {
    return m_pUnknownManager;
}

std::vector<KRResource *> KRContext::getResources()
{
    
    std::vector<KRResource *> resources;
    
    for(unordered_map<std::string, KRScene *>::iterator itr = m_pSceneManager->getScenes().begin(); itr != m_pSceneManager->getScenes().end(); itr++) {
        resources.push_back((*itr).second);
    }
    for(unordered_map<std::string, KRTexture *>::iterator itr = m_pTextureManager->getTextures().begin(); itr != m_pTextureManager->getTextures().end(); itr++) {
        resources.push_back((*itr).second);
    }
    for(unordered_map<std::string, KRMaterial *>::iterator itr = m_pMaterialManager->getMaterials().begin(); itr != m_pMaterialManager->getMaterials().end(); itr++) {
        resources.push_back((*itr).second);
    }
    for(unordered_multimap<std::string, KRMesh *>::iterator itr = m_pMeshManager->getModels().begin(); itr != m_pMeshManager->getModels().end(); itr++) {
        resources.push_back((*itr).second);
    }
    for(unordered_map<std::string, KRAnimation *>::iterator itr = m_pAnimationManager->getAnimations().begin(); itr != m_pAnimationManager->getAnimations().end(); itr++) {
        resources.push_back((*itr).second);
    }
    for(unordered_map<std::string, KRAnimationCurve *>::iterator itr = m_pAnimationCurveManager->getAnimationCurves().begin(); itr != m_pAnimationCurveManager->getAnimationCurves().end(); itr++) {
        resources.push_back((*itr).second);
    }
    for(unordered_map<std::string, KRAudioSample *>::iterator itr = m_pSoundManager->getSounds().begin(); itr != m_pSoundManager->getSounds().end(); itr++) {
        resources.push_back((*itr).second);
    }
    
    unordered_map<std::string, unordered_map<std::string, KRUnknown *> > unknowns = m_pUnknownManager->getUnknowns();
    for(unordered_map<std::string, unordered_map<std::string, KRUnknown *> >::iterator itr = unknowns.begin(); itr != unknowns.end(); itr++) {
        for(unordered_map<std::string, KRUnknown *>::iterator itr2 = (*itr).second.begin(); itr2 != (*itr).second.end(); itr2++) {
            resources.push_back((*itr2).second);
        }
    }
    
    // FINDME, TODO - Not yet exporting shaders, as they are currently only being used as standard Kraken assets.  In the future people may want their custom shaders to be exported.
    
    return resources;
}

void KRContext::loadResource(const std::string &file_name, KRDataBlock *data) {
    std::string name = KRResource::GetFileBase(file_name);
    std::string extension = KRResource::GetFileExtension(file_name);
    
//    fprintf(stderr, "KRContext::loadResource - Loading: %s\n", file_name.c_str());
    
    if(extension.compare("krbundle") == 0) {
        m_pBundleManager->loadBundle(name.c_str(), data);
    } else if(extension.compare("krmesh") == 0) {
        m_pMeshManager->loadModel(name.c_str(), data);
    } else if(extension.compare("krscene") == 0) {
        m_pSceneManager->loadScene(name.c_str(), data);
    } else if(extension.compare("kranimation") == 0) {
        m_pAnimationManager->loadAnimation(name.c_str(), data);
    } else if(extension.compare("kranimationcurve") == 0) {
        m_pAnimationCurveManager->loadAnimationCurve(name.c_str(), data);
    } else if(extension.compare("pvr") == 0) {
        m_pTextureManager->loadTexture(name.c_str(), extension.c_str(), data);
    } else if(extension.compare("ktx") == 0) {
        m_pTextureManager->loadTexture(name.c_str(), extension.c_str(), data);
    } else if(extension.compare("tga") == 0) {
        m_pTextureManager->loadTexture(name.c_str(), extension.c_str(), data);
    } else if(extension.compare("vsh") == 0) {
        m_pShaderManager->loadVertexShader(name.c_str(), data);
    } else if(extension.compare("fsh") == 0) {
        m_pShaderManager->loadFragmentShader(name.c_str(), data);
    } else if(extension.compare("mtl") == 0) {
        m_pMaterialManager->load(name.c_str(), data);
    } else if(extension.compare("mp3") == 0) {
        m_pSoundManager->load(name.c_str(), extension, data);
    } else if(extension.compare("wav") == 0) {
        m_pSoundManager->load(name.c_str(), extension, data);
    } else if(extension.compare("aac") == 0) {
        m_pSoundManager->load(name.c_str(), extension, data);
    } else if(extension.compare("obj") == 0) {
        KRResource::LoadObj(*this, file_name);
#if !TARGET_OS_IPHONE
    } else if(extension.compare("fbx") == 0) {
        KRResource::LoadFbx(*this, file_name);
    } else if(extension.compare("blend") == 0) {
        KRResource::LoadBlenderScene(*this, file_name);
#endif
    } else {
        m_pUnknownManager->load(name, extension, data);
    }
}

void KRContext::loadResource(std::string path) {
    KRDataBlock *data = new KRDataBlock();
    if(data->load(path)) {
        loadResource(path, data);
    } else {
        KRContext::Log(KRContext::LOG_LEVEL_ERROR, "KRContext::loadResource - Failed to open file: %s", path.c_str());
        delete data;
    }
}

void KRContext::detectExtensions() {
    m_bDetectedExtensions = true;
    
}

void KRContext::startFrame(float deltaTime)
{
    m_streamer.startStreamer();
    m_pTextureManager->startFrame(deltaTime);
    m_pAnimationManager->startFrame(deltaTime);
    m_pSoundManager->startFrame(deltaTime);
    m_pMeshManager->startFrame(deltaTime);
}

void KRContext::endFrame(float deltaTime)
{
    m_pTextureManager->endFrame(deltaTime);
    m_pAnimationManager->endFrame(deltaTime);
    m_pMeshManager->endFrame(deltaTime);
    m_current_frame++;
    m_absolute_time += deltaTime;
}

long KRContext::getCurrentFrame() const
{
    return m_current_frame;
}

long KRContext::getLastFullyStreamedFrame() const
{
    return m_last_fully_streamed_frame;
}

float KRContext::getAbsoluteTime() const
{
    return m_absolute_time;
}


long KRContext::getAbsoluteTimeMilliseconds()
{
#ifdef __APPLE__
    return (long)(mach_absolute_time() / 1000 * m_timebase_info.numer / m_timebase_info.denom); // Division done first to avoid potential overflow
#else
    return (long)GetTickCount64();
#endif
}

bool KRContext::getStreamingEnabled()
{
    return m_streamingEnabled;
}

void KRContext::setStreamingEnabled(bool enable)
{
    m_streamingEnabled = enable;
}


#if TARGET_OS_IPHONE || TARGET_OS_MAC

void KRContext::getMemoryStats(long &free_memory)
{
    free_memory = 0;

    mach_port_t host_port = mach_host_self();
    mach_msg_type_number_t host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
    vm_size_t pagesize = 0;
    vm_statistics_data_t vm_stat;
    // int total_ram = 256 * 1024 * 1024;
    if(host_page_size(host_port, &pagesize) != KERN_SUCCESS) {
        KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Could not get VM page size.");
    } else if(host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) != KERN_SUCCESS) {
        KRContext::Log(KRContext::LOG_LEVEL_ERROR, "Could not get VM stats.");
    } else {
        // total_ram = (vm_stat.wire_count + vm_stat.active_count + vm_stat.inactive_count + vm_stat.free_count) * pagesize;
        
        free_memory = (vm_stat.free_count + vm_stat.inactive_count) * pagesize;
    }
}

#endif

void KRContext::doStreaming()
{
    if(m_streamingEnabled) {
        /*
        long free_memory = KRENGINE_GPU_MEM_TARGET;
        long total_memory = KRENGINE_GPU_MEM_MAX;
        */
        /*
#if TARGET_OS_IPHONE
        // FINDME, TODO, HACK! - Experimental code, need to expose through engine parameters
        const long KRENGINE_RESERVE_MEMORY = 0x4000000; // 64MB
        
        getMemoryStats(free_memory);
        free_memory = KRCLAMP(free_memory - KRENGINE_RESERVE_MEMORY, 0, KRENGINE_GPU_MEM_TARGET);
        total_memory = KRMIN(KRENGINE_GPU_MEM_MAX, free_memory * 3 / 4 + m_pTextureManager->getMemUsed() + m_pMeshManager->getMemUsed());
        
#endif
        */
        /*
        // FINDME, TODO - Experimental code, need to expose through engine parameters
        const long MEMORY_WARNING_THROTTLE_FRAMES = 5;
        bool memory_warning_throttle = m_last_memory_warning_frame != 0 && m_current_frame - m_last_memory_warning_frame < MEMORY_WARNING_THROTTLE_FRAMES;
        if(memory_warning_throttle) {
            free_memory = 0;
        }
        */
        
        /*
        // FINDME, TODO - Experimental code, need to expose through engine parameters
        const long MEMORY_WARNING_THROTTLE2_FRAMES = 30;
        bool memory_warning_throttle2 = m_last_memory_warning_frame != 0 && m_current_frame - m_last_memory_warning_frame < MEMORY_WARNING_THROTTLE2_FRAMES;
        if(memory_warning_throttle2) {
            total_memory /= 2;
            free_memory /= 2;
        }
        */
        
        /*
        m_pMeshManager->doStreaming(total_memory, free_memory);
        m_pTextureManager->doStreaming(total_memory, free_memory);
        */
        
        
        long streaming_start_frame = m_current_frame;
        
        long memoryRemaining = KRENGINE_GPU_MEM_TARGET;
        long memoryRemainingThisFrame = KRENGINE_GPU_MEM_MAX - m_pTextureManager->getMemUsed() - m_pMeshManager->getMemUsed();
        long memoryRemainingThisFrameStart = memoryRemainingThisFrame;
        m_pMeshManager->doStreaming(memoryRemaining, memoryRemainingThisFrame);
        m_pTextureManager->doStreaming(memoryRemaining, memoryRemainingThisFrame);
        
        if(memoryRemainingThisFrame == memoryRemainingThisFrameStart && memoryRemainingThisFrame > 0) {
            m_last_fully_streamed_frame = streaming_start_frame;
        }
        
    }
}

void KRContext::receivedMemoryWarning()
{
    m_last_memory_warning_frame = m_current_frame;
}


