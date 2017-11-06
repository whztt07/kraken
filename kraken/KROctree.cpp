//
//  KROctree.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-29.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "public/kraken.h"
#include "KROctree.h"
#include "KRNode.h"
#include "KRCollider.h"

KROctree::KROctree()
{
    m_pRootNode = NULL;
}

KROctree::~KROctree()
{
    if(m_pRootNode) {
        delete m_pRootNode;
    }
}

void KROctree::add(KRNode *pNode)
{
    AABB nodeBounds = pNode->getBounds();
    if(nodeBounds == AABB::Zero()) {
        // This item is not visible, don't add it to the octree or outer scene nodes
    } else if(nodeBounds == AABB::Infinite()) {
        // This item is infinitely large; we track it separately
        m_outerSceneNodes.insert(pNode);
    } else { 
        if(m_pRootNode == NULL) {
            // First item inserted, create a node large enough to fit it
            m_pRootNode = new KROctreeNode(NULL, nodeBounds);
            m_pRootNode->add(pNode);
        } else {
            // Keep encapsulating the root node until the new root contains the inserted node
            bool bInsideRoot = false;
            while(!bInsideRoot) {
                AABB rootBounds = m_pRootNode->getBounds();
                Vector3 rootSize = rootBounds.size();
                if(nodeBounds.min.x < rootBounds.min.x || nodeBounds.min.y < rootBounds.min.y || nodeBounds.min.z < rootBounds.min.z) {
                    m_pRootNode = new KROctreeNode(NULL, AABB(rootBounds.min - rootSize, rootBounds.max), 7, m_pRootNode);
                } else if(nodeBounds.max.x > rootBounds.max.x || nodeBounds.max.y > rootBounds.max.y || nodeBounds.max.z > rootBounds.max.z) {
                    m_pRootNode = new KROctreeNode(NULL, AABB(rootBounds.min, rootBounds.max + rootSize), 0, m_pRootNode);
                } else {
                    bInsideRoot = true;
                }
            }
            m_pRootNode->add(pNode);
        }
    }
}

void KROctree::remove(KRNode *pNode)
{
    if(!m_outerSceneNodes.erase(pNode)) {
        if(m_pRootNode) {
            pNode->removeFromOctreeNodes();
        }
    }
    
    shrink();
}

void KROctree::update(KRNode *pNode)
{
    // TODO: This may be more efficient as an incremental operation rather than removing and re-adding the node
    remove(pNode);
    add(pNode);
    shrink();
}

void KROctree::shrink()
{
    if(m_pRootNode) {
        while(m_pRootNode->canShrinkRoot()) {
            KROctreeNode *newRoot = m_pRootNode->stripChild();
            delete m_pRootNode;
            m_pRootNode = newRoot;
            if(m_pRootNode == NULL) return;
        }
    }
}

KROctreeNode *KROctree::getRootNode()
{
    return m_pRootNode;
}

std::set<KRNode *> &KROctree::getOuterSceneNodes()
{
    return m_outerSceneNodes;
}


bool KROctree::lineCast(const Vector3 &v0, const Vector3 &v1, HitInfo &hitinfo, unsigned int layer_mask)
{
    bool hit_found = false;
    std::vector<KRCollider *> outer_colliders;
    
    for(std::set<KRNode *>::iterator outer_nodes_itr=m_outerSceneNodes.begin(); outer_nodes_itr != m_outerSceneNodes.end(); outer_nodes_itr++) {
        KRCollider *collider = dynamic_cast<KRCollider *>(*outer_nodes_itr);
        if(collider) {
            outer_colliders.push_back(collider);
        }
    }
    for(std::vector<KRCollider *>::iterator itr=outer_colliders.begin(); itr != outer_colliders.end(); itr++) {
        if((*itr)->lineCast(v0, v1, hitinfo, layer_mask)) hit_found = true;
    }
    
    if(m_pRootNode) {
        if(m_pRootNode->lineCast(v0, v1, hitinfo, layer_mask)) hit_found = true;
    }
    return hit_found;
}

bool KROctree::rayCast(const Vector3 &v0, const Vector3 &dir, HitInfo &hitinfo, unsigned int layer_mask)
{
    bool hit_found = false;
    for(std::set<KRNode *>::iterator outer_nodes_itr=m_outerSceneNodes.begin(); outer_nodes_itr != m_outerSceneNodes.end(); outer_nodes_itr++) {
        KRCollider *collider = dynamic_cast<KRCollider *>(*outer_nodes_itr);
        if(collider) {
            if(collider->rayCast(v0, dir, hitinfo, layer_mask)) hit_found = true;
        }
    }
    if(m_pRootNode) {
        if(m_pRootNode->rayCast(v0, dir, hitinfo, layer_mask)) hit_found = true;
    }
    return hit_found;
}

bool KROctree::sphereCast(const Vector3 &v0, const Vector3 &v1, float radius, HitInfo &hitinfo, unsigned int layer_mask)
{
    bool hit_found = false;
    std::vector<KRCollider *> outer_colliders;
    
    for(std::set<KRNode *>::iterator outer_nodes_itr=m_outerSceneNodes.begin(); outer_nodes_itr != m_outerSceneNodes.end(); outer_nodes_itr++) {
        KRCollider *collider = dynamic_cast<KRCollider *>(*outer_nodes_itr);
        if(collider) {
            outer_colliders.push_back(collider);
        }
    }
    for(std::vector<KRCollider *>::iterator itr=outer_colliders.begin(); itr != outer_colliders.end(); itr++) {
        if((*itr)->sphereCast(v0, v1, radius, hitinfo, layer_mask)) hit_found = true;
    }
    
    if(m_pRootNode) {
        if(m_pRootNode->sphereCast(v0, v1, radius, hitinfo, layer_mask)) hit_found = true;
    }
    return hit_found;
}

