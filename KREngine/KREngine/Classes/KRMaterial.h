//
//  KRMaterial.h
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

#import <stdint.h>
#import <list>
#import <string>

#import "KREngine-common.h"

using std::list;

#import "KRTexture.h"
#import "KRShaderManager.h"
#import "KRShader.h"
#import "KRCamera.h"
#import "KRResource.h"
#import "KRVector2.h"
#import "KRScene.h"

#ifndef KRMATERIAL_H
#define KRMATERIAL_H



class KRTextureManager;
class KRContext;

class KRMaterial : public KRResource {
public:
    typedef enum {
        KRMATERIAL_ALPHA_MODE_OPAQUE, // Non-transparent materials
        KRMATERIAL_ALPHA_MODE_TEST, // Alpha in diffuse texture is interpreted as punch-through when < 0.5
        KRMATERIAL_ALPHA_MODE_BLENDONESIDE, // Blended alpha with backface culling
        KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE // Blended alpha rendered in two passes.  First pass renders backfaces; second pass renders frontfaces.
    } alpha_mode_type;
    
    KRMaterial(KRContext &context, const char *szName);
    virtual ~KRMaterial();
    
    virtual std::string getExtension();
    virtual bool save(const std::string& path);
    
    
    void setAmbientMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset);
    void setDiffuseMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset);
    void setSpecularMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset);
    void setReflectionMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset);
    void setReflectionCube(std::string texture_name);
    void setNormalMap(std::string texture_name, KRVector2 texture_scale, KRVector2 texture_offset);
    void setAmbient(const KRVector3 &c);
    void setDiffuse(const KRVector3 &c);    
    void setSpecular(const KRVector3 &c);
    void setReflection(const KRVector3 &c);
    void setTransparency(GLfloat a);
    void setShininess(GLfloat s);
    void setReflectionFactor(GLfloat r);
    void setAlphaMode(alpha_mode_type blend_mode);
    alpha_mode_type getAlphaMode();
    

    bool isTransparent();
    char *getName();
    
#if TARGET_OS_IPHONE
    bool bind(KRMaterial **prevBoundMaterial, char *szPrevShaderKey, KRCamera *pCamera, KRMat4 &matModelToView, KRMat4 &mvpMatrix, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRContext *pContext, KRTexture *pLightMap, KRNode::RenderPass renderPass);
    
#endif
    

    
private:
    char m_szName[256];
    
    KRTexture *m_pAmbientMap; // mtl map_Ka value
    KRTexture *m_pDiffuseMap; // mtl map_Kd value
    KRTexture *m_pSpecularMap; // mtl map_Ks value
    KRTexture *m_pReflectionMap; // mtl refl value
    KRTexture *m_pReflectionCube;
    KRTexture *m_pNormalMap; // mtl map_Normal value
    std::string m_ambientMap;
    std::string m_diffuseMap;
    std::string m_specularMap;
    std::string m_reflectionMap;
    std::string m_reflectionCube;
    std::string m_normalMap;
    
    KRVector2 m_ambientMapScale;
    KRVector2 m_ambientMapOffset;
    KRVector2 m_diffuseMapScale;
    KRVector2 m_diffuseMapOffset;
    KRVector2 m_specularMapScale;
    KRVector2 m_specularMapOffset;
    KRVector2 m_reflectionMapScale;
    KRVector2 m_reflectionMapOffset;
    KRVector2 m_normalMapScale;
    KRVector2 m_normalMapOffset;
    
    KRVector3 m_ambientColor; // Ambient rgb
    KRVector3 m_diffuseColor; // Diffuse rgb
    KRVector3 m_specularColor; // Specular rgb
    KRVector3 m_reflectionColor; // Reflection rgb
    
    //GLfloat m_ka_r, m_ka_g, m_ka_b; // Ambient rgb
    //GLfloat m_kd_r, m_kd_g, m_kd_b; // Diffuse rgb
    //GLfloat m_ks_r, m_ks_g, m_ks_b; // Specular rgb
    //GLfloat m_kr_r, m_kr_g, m_kr_b; // Reflection rgb
    
    GLfloat m_tr; // Transparency
    GLfloat m_ns; // Shininess
    GLfloat m_reflectionFactor; // Level of reflectivity
    
    alpha_mode_type m_alpha_mode;
};

#endif
