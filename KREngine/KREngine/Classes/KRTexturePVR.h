//
//  KRTexturePVR.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-10-23.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRTEXTUREPVR_H
#define KRTEXTUREPVR_H

#include "KRTexture2D.h"

class KRTexturePVR : public KRTexture2D
{
public:
    KRTexturePVR(KRContext &context, KRDataBlock *data);
    virtual ~KRTexturePVR();
    
    bool uploadTexture(GLenum target, int lod_max_dim, int &current_lod_max_dim, size_t &textureMemUsed);
    
protected:
    
    uint32_t  m_iWidth;
    uint32_t  m_iHeight;
    GLenum    m_internalFormat;
    bool      m_bHasAlpha;
    
    struct    dataBlockStruct {
        void *start;
        uint32_t length;
    };
    
    std::list<dataBlockStruct> m_blocks;
};

#endif
