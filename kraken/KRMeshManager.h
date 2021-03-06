//
//  KRMeshManager.h
//  KREngine
//
//  Copyright 2012 Kearwood Gilbert. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without modification, are
//  permitted provided that the following conditions are met:
//  
//  1. Redistributions of source code must retain the above copyright notice, this list of
//  conditions and the following disclaimer.
//  
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//  of conditions and the following disclaimer in the documentation and/or other materials
//  provided with the distribution.
//  
//  THIS SOFTWARE IS PROVIDED BY KEARWOOD GILBERT ''AS IS'' AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KEARWOOD GILBERT OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
//  The views and conclusions contained in the software and documentation are those of the
//  authors and should not be interpreted as representing official policies, either expressed
//  or implied, of Kearwood Gilbert.
//

#ifndef KRMESHMANAGER_H
#define KRMESHMANAGER_H

#include "KREngine-common.h"
#include "KRContextObject.h"
#include "KRDataBlock.h"
#include "KRNode.h"

class KRContext;
class KRMesh;

class KRMeshManager : public KRContextObject {
public:
    static const int KRENGINE_MAX_VOLUMETRIC_PLANES=500;
    static const int KRENGINE_MAX_RANDOM_PARTICLES=150000;
    
    KRMeshManager(KRContext &context);
    virtual ~KRMeshManager();
    
    void startFrame(float deltaTime);
    void endFrame(float deltaTime);
    void firstFrame();
    
    KRMesh *loadModel(const char *szName, KRDataBlock *pData);
    std::vector<KRMesh *> getModel(const char *szName);
    void addModel(KRMesh *model);
    
    std::vector<std::string> getModelNames();
    unordered_multimap<std::string, KRMesh *> &getModels();
    
    class KRVBOData {
        
    public:
        
        typedef enum {
            STREAMING,
            CONSTANT,
            TEMPORARY
        } vbo_type;
        
        KRVBOData();
        KRVBOData(KRMeshManager *manager, KRDataBlock &data, KRDataBlock &index_data, int vertex_attrib_flags, bool static_vbo, vbo_type t);
        void init(KRMeshManager *manager, KRDataBlock &data, KRDataBlock &index_data, int vertex_attrib_flags, bool static_vbo, vbo_type t);
        ~KRVBOData();
        
        
        KRDataBlock *m_data;
        KRDataBlock *m_index_data;
        
        bool isVBOLoaded() { return m_is_vbo_loaded; }
        bool isVBOReady() { return m_is_vbo_ready; }
        void load();
        void unload();
        void bind();
        
        // Disable copy constructors
        KRVBOData(const KRVBOData& o) = delete;
        KRVBOData& operator=(const KRVBOData& o) = delete;
        
        long getSize() { return m_size; }
        
        void resetPoolExpiry(float lodCoverage);
        long getLastFrameUsed() { return m_last_frame_used; }
        
        vbo_type getType() { return m_type; }
        
        float getStreamPriority();
        
        void _swapHandles();
        
    private:
        KRMeshManager *m_manager;
        int m_vertex_attrib_flags;
        GLuint m_vbo_handle;
        GLuint m_vbo_handle_indexes;
        GLuint m_vao_handle;
        GLsizeiptr m_size;
        
        long m_last_frame_used;
        float m_last_frame_max_lod_coverage;
        vbo_type m_type;
        bool m_static_vbo;
        bool m_is_vbo_loaded;
        bool m_is_vbo_ready;
    };
    
    void bindVBO(KRVBOData *vbo_data, float lodCoverage);
    void bindVBO(KRDataBlock &data, KRDataBlock &index_data, int vertex_attrib_flags, bool static_vbo, float lodCoverage);
    void unbindVBO();
    long getMemUsed();
    long getMemActive();
    
    static void configureAttribs(__int32_t attributes);
    
    typedef struct {
        GLfloat x;
        GLfloat y;
        GLfloat z;
    } Vector3D;
    
    typedef struct {
        GLfloat u;
        GLfloat v;
    } TexCoord;
    
    typedef struct {
        Vector3D vertex;
        TexCoord uva;
    } RandomParticleVertexData;
    
    typedef struct {
        Vector3D vertex;
    } VolumetricLightingVertexData;
    

    KRDataBlock &getRandomParticles();
    KRDataBlock &getVolumetricLightingVertexes();
    
    
    long getMemoryTransferedThisFrame();

    int getActiveVBOCount();
    
    struct draw_call_info {
        KRNode::RenderPass pass;
        char object_name[256];
        char material_name[256];
        int vertex_count;
    };
    
    void log_draw_call(KRNode::RenderPass pass, const std::string &object_name, const std::string &material_name, int vertex_count);
    std::vector<draw_call_info> getDrawCalls();
    
    

    KRVBOData KRENGINE_VBO_DATA_3D_CUBE_VERTICES;
    KRVBOData KRENGINE_VBO_DATA_2D_SQUARE_VERTICES;
    
    void doStreaming(long &memoryRemaining, long &memoryRemainingThisFrame);
    
private:
    KRDataBlock KRENGINE_VBO_3D_CUBE_VERTICES, KRENGINE_VBO_3D_CUBE_INDEXES;
    __int32_t KRENGINE_VBO_3D_CUBE_ATTRIBS;
    KRDataBlock KRENGINE_VBO_2D_SQUARE_VERTICES, KRENGINE_VBO_2D_SQUARE_INDEXES;
    __int32_t KRENGINE_VBO_2D_SQUARE_ATTRIBS;
    
    unordered_multimap<std::string, KRMesh *> m_models; // Multiple models with the same name/key may be inserted, representing multiple LOD levels of the model
    
    long m_vboMemUsed;
    KRVBOData *m_currentVBO;
    
    unordered_map<KRDataBlock *, KRVBOData *> m_vbosActive;
    std::vector<std::pair<float, KRVBOData *> > m_activeVBOs_streamer;
    std::vector<std::pair<float, KRVBOData *> > m_activeVBOs_streamer_copy;
    
    KRDataBlock m_randomParticleVertexData;
    KRDataBlock m_volumetricLightingVertexData;
    
    long m_memoryTransferredThisFrame;
    
    std::vector<draw_call_info> m_draw_calls;
    bool m_draw_call_logging_enabled;
    bool m_draw_call_log_used;
    
    bool m_first_frame;
    
    std::mutex m_streamerFenceMutex;
    bool m_streamerComplete;
    
    void balanceVBOMemory(long &memoryRemaining, long &memoryRemainingThisFrame);
    
    void primeVBO(KRVBOData *vbo_data);

};

#endif
