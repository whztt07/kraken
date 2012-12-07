//
//  KRAnimationCurve.cpp
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

#include "KRAnimationCurve.h"

KRAnimationCurve::KRAnimationCurve(KRContext &context, std::string name) : KRResource(context, name)
{
    m_pData = new KRDataBlock();
    m_pData->expand(sizeof(animation_curve_header));
    animation_curve_header *header = (animation_curve_header *)m_pData->getStart();
    strcpy(header->szTag, "KRCURVE1.0     ");
    header->frame_rate = 30.0f;
    header->frame_start = 0;
    header->frame_count = 0;
}

KRAnimationCurve::~KRAnimationCurve()
{
    m_pData->unload();
    delete m_pData;
}

bool KRAnimationCurve::load(KRDataBlock *data)
{
    m_pData->unload();
    delete m_pData;
    m_pData = data;
    return true;
}

std::string KRAnimationCurve::getExtension() {
    return "kranimationcurve";
}

bool KRAnimationCurve::save(const std::string& path) {
    return m_pData->save(path);
}


KRAnimationCurve *KRAnimationCurve::Load(KRContext &context, const std::string &name, KRDataBlock *data)
{
    KRAnimationCurve *new_animation_curve = new KRAnimationCurve(context, name);
    if(new_animation_curve->load(data)) {
        return new_animation_curve;
    } else {
        delete new_animation_curve;
        delete data;
        return NULL;
    }
}

int KRAnimationCurve::getFrameCount()
{
    return ((animation_curve_header *)m_pData->getStart())->frame_count;
}

void KRAnimationCurve::setFrameCount(int frame_count)
{
    int prev_frame_count = getFrameCount();
    if(frame_count != prev_frame_count) {
        float fill_value = 0.0f;
        if(prev_frame_count > 0) {
            fill_value = getValue(prev_frame_count - 1);
        }
        m_pData->expand(sizeof(animation_curve_header) + sizeof(float) * frame_count - m_pData->getSize());
        float *frame_data = (float *)((char *)m_pData->getStart() + sizeof(animation_curve_header));
        for(int frame_number=prev_frame_count; frame_number < frame_count; frame_number++) {
            frame_data[frame_number] = fill_value;
        }
        ((animation_curve_header *)m_pData->getStart())->frame_count = frame_count;
    }
}

float KRAnimationCurve::getFrameRate()
{
    return ((animation_curve_header *)m_pData->getStart())->frame_rate;
}

void KRAnimationCurve::setFrameRate(float frame_rate)
{
    ((animation_curve_header *)m_pData->getStart())->frame_rate = frame_rate;
}

int KRAnimationCurve::getFrameStart()
{
    return ((animation_curve_header *)m_pData->getStart())->frame_start;
}

void KRAnimationCurve::setFrameStart(int frame_number)
{
    ((animation_curve_header *)m_pData->getStart())->frame_start = frame_number;
}

float KRAnimationCurve::getValue(int frame_number)
{
    int clamped_frame = frame_number;
    if(frame_number < 0) {
        clamped_frame = frame_number;
    } else if(frame_number > getFrameCount()) {
        clamped_frame = getFrameCount();
    }
    float *frame_data = (float *)((char *)m_pData->getStart() + sizeof(animation_curve_header));
    return frame_data[clamped_frame];
}

void KRAnimationCurve::setValue(int frame_number, float value)
{
    if(frame_number >= 0 && frame_number < getFrameCount()) {
        float *frame_data = (float *)((char *)m_pData->getStart() + sizeof(animation_curve_header));
        frame_data[frame_number] = value;
    }
}

