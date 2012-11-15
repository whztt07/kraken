//
//  KRNode.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-11.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRNODE_H
#define KRNODE_H

#import "KRResource.h"
#import "KRVector3.h"
#import "KRViewport.h"
#import "tinyxml2.h"
#import <stack.h>

class KRCamera;
class KRShaderManager;
class KRModelManager;
class KRMaterialManager;
class KRMat4;
class KRTextureManager;
class KRContext;
class KRScene;
class KRAABB;

class KRNode : public KRContextObject
{
public:
    enum RenderPass {
        RENDER_PASS_FORWARD_OPAQUE,
        RENDER_PASS_DEFERRED_GBUFFER,
        RENDER_PASS_DEFERRED_LIGHTS,
        RENDER_PASS_DEFERRED_OPAQUE,
        RENDER_PASS_FORWARD_TRANSPARENT,
        RENDER_PASS_ADDITIVE_PARTICLES,
        RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE,
        RENDER_PASS_GENERATE_SHADOWMAPS,
        RENDER_PASS_SHADOWMAP
    };
    
    KRNode(KRScene &scene, std::string name);
    virtual ~KRNode();
    
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    static KRNode *LoadXML(KRScene &scene, tinyxml2::XMLElement *e);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    virtual std::string getElementName();
    const std::string &getName();
    
    void addChild(KRNode *child);
    const std::vector<KRNode *> &getChildren();
    
    void setLocalTranslation(const KRVector3 &v);
    void setLocalScale(const KRVector3 &v);
    void setLocalRotation(const KRVector3 &v);
    
    const KRVector3 &getLocalTranslation();
    const KRVector3 &getLocalScale();
    const KRVector3 &getLocalRotation();
    
    const KRVector3 &getWorldTranslation();
    const KRVector3 &getWorldScale();
    const KRVector3 &getWorldRotation();
    
    virtual KRAABB getBounds();
    const KRMat4 &getModelMatrix();
    
    KRScene &getScene();
#if TARGET_OS_IPHONE
    
    virtual void render(KRCamera *pCamera, std::stack<KRLight *> &lights, const KRViewport &viewport, RenderPass renderPass);

#endif
    
    virtual void physicsUpdate(float deltaTime);
    virtual bool hasPhysics();
    
protected:
    KRVector3 m_localTranslation;
    KRVector3 m_localScale;
    KRVector3 m_localRotation;
    
private:
    void invalidateModelMatrix();
    KRMat4 m_modelMatrix;
    bool m_modelMatrixValid;
    
    std::string m_name;
    
    std::vector<KRNode *> m_childNodes;
    KRNode *m_parentNode;
    
    KRScene *m_pScene;
    
};


#endif
