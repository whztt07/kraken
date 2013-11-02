//
//  KRLODSet
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRLODSET_H
#define KRLODSET_H

#include "KRResource.h"
#include "KRNode.h"

class KRLODSet : public KRNode {
public:
    KRLODSet(KRScene &scene, std::string name);
    virtual ~KRLODSet();
    virtual std::string getElementName();
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
};


#endif
