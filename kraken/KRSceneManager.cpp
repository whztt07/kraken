//
//  KRSceneManager.cpp
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

#include "KRSceneManager.h"
#include "KRScene.h"

KRSceneManager::KRSceneManager(KRContext &context) : KRContextObject(context){
}

KRSceneManager::~KRSceneManager() {
    for(unordered_map<std::string, KRScene *>::iterator itr = m_scenes.begin(); itr != m_scenes.end(); ++itr){
        delete (*itr).second;
    }
    m_scenes.empty();
}

KRScene *KRSceneManager::loadScene(const std::string &name, KRDataBlock *data) {
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    
    KRScene *pScene = KRScene::Load(*m_pContext, name, data);
    m_scenes[lowerName] = pScene;
    return pScene;
}

void KRSceneManager::add(KRScene *scene)
{
    std::string lowerName = scene->getName();
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    m_scenes[lowerName] = scene;
}

KRScene *KRSceneManager::getScene(const std::string &name) {
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    
    static unordered_map<std::string, KRScene *>::iterator scene_itr = m_scenes.find(lowerName);
    if(scene_itr != m_scenes.end()) {
        return (*scene_itr).second;
    } else {
        return NULL;
    }
}

KRScene *KRSceneManager::getFirstScene() {
    static unordered_map<std::string, KRScene *>::iterator scene_itr = m_scenes.begin();
    if(scene_itr != m_scenes.end()) {
        return (*scene_itr).second;
    } else {
        return NULL;
    }
}

unordered_map<std::string, KRScene *> &KRSceneManager::getScenes() {
    return m_scenes;
}

