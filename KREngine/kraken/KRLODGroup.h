//
//  KRLODGroup.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRLODGROUP_H
#define KRLODGROUP_H

#include "KRResource.h"
#include "KRNode.h"
#include "KRTexture.h"

class KRLODGroup : public KRNode {
public:
    KRLODGroup(KRScene &scene, std::string name);
    virtual ~KRLODGroup();
    virtual std::string getElementName();
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    virtual void updateLODVisibility(const KRViewport &viewport);
    
    float getMinDistance();
    float getMaxDistance();
    void setMinDistance(float min_distance);
    void setMaxDistance(float max_distance);
    
private:
    bool getLODVisibility(const KRViewport &viewport);
    float m_min_distance;
    float m_max_distance;
};


#endif