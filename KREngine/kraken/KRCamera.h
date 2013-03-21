//
//  KRSettings.h
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

#ifndef KRCAMERA_H
#define KRCAMERA_H

#include "KREngine-common.h"

#include "KRMat4.h"
#include "KRVector2.h"
#include "KRAABB.h"
#include "KRShader.h"
#include "KRContextObject.h"
#include "KRTexture.h"
#include "KRContext.h"
#include "KRViewport.h"
#include "KRRenderSettings.h"

class KRModel;
class KRScene;
class KRViewport;

class KRCamera : public KRNode {
public:
    KRCamera(KRScene &scene, std::string name);
    virtual ~KRCamera();

    void renderFrame(float deltaTime, GLint renderBufferWidth, GLint renderBufferHeight);
    
    KRRenderSettings settings;
        
private:    
    void createBuffers(GLint renderBufferWidth, GLint renderBufferHeight);
    
    GLint backingWidth, backingHeight;
    GLint volumetricBufferWidth, volumetricBufferHeight;
    
    GLuint compositeFramebuffer, compositeDepthTexture, compositeColorTexture;
    GLuint lightAccumulationBuffer, lightAccumulationTexture;
    
    
    GLuint volumetricLightAccumulationBuffer, volumetricLightAccumulationTexture;
    
    void renderPost();
        
    void destroyBuffers();
    
    KRTexture *m_pSkyBoxTexture;
    KRViewport m_viewport;
    
    float m_particlesAbsoluteTime;
    
    
    typedef struct {
        GLfloat x;
        GLfloat y;
        GLfloat z;
        GLfloat u;
        GLfloat v;
    } DebugTextVertexData;
    
    DebugTextVertexData *m_debug_text_vertices;
};

#endif
