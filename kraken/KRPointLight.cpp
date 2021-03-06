//
//  KRPointLight.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KREngine-common.h"

#include "KRPointLight.h"
#include "KRCamera.h"
#include "KRContext.h"
#include "KRStockGeometry.h"

KRPointLight::KRPointLight(KRScene &scene, std::string name) : KRLight(scene, name)
{
    m_sphereVertices = NULL;
    m_cVertices = 0;
}

KRPointLight::~KRPointLight()
{
    if(m_sphereVertices) {
        delete m_sphereVertices;
        m_cVertices = 0;
    }
}

std::string KRPointLight::getElementName() {
    return "point_light";
}

AABB KRPointLight::getBounds() {
    float influence_radius = m_decayStart - sqrt(m_intensity * 0.01f) / sqrt(KRLIGHT_MIN_INFLUENCE);
    if(influence_radius < m_flareOcclusionSize) {
        influence_radius = m_flareOcclusionSize;
    }
    return AABB::Create(Vector3::Create(-influence_radius), Vector3::Create(influence_radius), getModelMatrix());
}

void KRPointLight::render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass)
{
    if(m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;
    
    KRLight::render(pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass);
    
    bool bVisualize = renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT && pCamera->settings.bShowDeferred;
    
    if(renderPass == KRNode::RENDER_PASS_DEFERRED_LIGHTS || bVisualize) {
        // Lights are rendered on the second pass of the deferred renderer
        
        std::vector<KRPointLight *> this_light;
        this_light.push_back(this);

        Vector3 light_position = getLocalTranslation();
        
        float influence_radius = m_decayStart - sqrt(m_intensity * 0.01f) / sqrt(KRLIGHT_MIN_INFLUENCE);
        
        Matrix4 sphereModelMatrix = Matrix4();
        sphereModelMatrix.scale(influence_radius);
        sphereModelMatrix.translate(light_position.x, light_position.y, light_position.z);

        if(viewport.visible(getBounds())) { // Cull out any lights not within the view frustrum

            Vector3 view_light_position = Matrix4::Dot(viewport.getViewMatrix(), light_position);
            
            bool bInsideLight = view_light_position.sqrMagnitude() <= (influence_radius + pCamera->settings.getPerspectiveNearZ()) * (influence_radius + pCamera->settings.getPerspectiveNearZ());
            
            KRShader *pShader = getContext().getShaderManager()->getShader(bVisualize ? "visualize_overlay" : (bInsideLight ? "light_point_inside" : "light_point"), pCamera, this_light, std::vector<KRDirectionalLight *>(), std::vector<KRSpotLight *>(), 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
            if(getContext().getShaderManager()->selectShader(*pCamera, pShader, viewport, sphereModelMatrix, this_light, std::vector<KRDirectionalLight *>(), std::vector<KRSpotLight *>(), 0, renderPass, Vector3::Zero(), 0.0f, Vector4::Zero())) {
                
                
                pShader->setUniform(KRShader::KRENGINE_UNIFORM_LIGHT_COLOR, m_color);
                pShader->setUniform(KRShader::KRENGINE_UNIFORM_LIGHT_INTENSITY, m_intensity * 0.01f);
                pShader->setUniform(KRShader::KRENGINE_UNIFORM_LIGHT_DECAY_START, getDecayStart());
                pShader->setUniform(KRShader::KRENGINE_UNIFORM_LIGHT_CUTOFF, KRLIGHT_MIN_INFLUENCE);                
                pShader->setUniform(KRShader::KRENGINE_UNIFORM_LIGHT_POSITION, light_position);

                
                if(bVisualize) {
                    // Enable additive blending
                    GLDEBUG(glEnable(GL_BLEND));
                    GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
                }
                
                // Disable z-buffer write
                GLDEBUG(glDepthMask(GL_FALSE));
                
                
                if(bInsideLight) {
                    
                    // Disable z-buffer test
                    GLDEBUG(glDisable(GL_DEPTH_TEST));
                    
                    // Render a full screen quad
                    m_pContext->getMeshManager()->bindVBO(&m_pContext->getMeshManager()->KRENGINE_VBO_DATA_2D_SQUARE_VERTICES, 1.0f);
                    GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
                } else {
#if GL_OES_vertex_array_object
                    GLDEBUG(glBindVertexArrayOES(0));
#endif
                    m_pContext->getMeshManager()->configureAttribs(1 << KRMesh::KRENGINE_ATTRIB_VERTEX);
                    // Render sphere of light's influence
                    generateMesh();
                
                    // Enable z-buffer test
                    GLDEBUG(glEnable(GL_DEPTH_TEST));
                    GLDEBUG(glDepthFunc(GL_LEQUAL));
                    GLDEBUG(glDepthRangef(0.0, 1.0));
                    
                    GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, m_sphereVertices));
                    GLDEBUG(glDrawArrays(GL_TRIANGLES, 0, m_cVertices));
                }
            }
            if(bVisualize) {
                // Enable alpha blending
                GLDEBUG(glEnable(GL_BLEND));
                GLDEBUG(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
            }
        }
    }
}

void KRPointLight::generateMesh() {
    // Create a triangular facet approximation to a sphere
    // Based on algorithm from Paul Bourke: http://paulbourke.net/miscellaneous/sphere_cylinder/
    
    int iterations = 3;
    int facet_count = pow(4, iterations) * 8;
    
    if(m_cVertices != facet_count * 3) {
        if(m_sphereVertices) {
            free(m_sphereVertices);
            m_sphereVertices = NULL;
        }
        
        m_cVertices = facet_count * 3;

        
        class Facet3 {
        public:
            Facet3() {
                
            }
            ~Facet3() {
                
            }
            Vector3 p1;
            Vector3 p2;
            Vector3 p3;
        };
        
        std::vector<Facet3> f = std::vector<Facet3>(facet_count);
        
        int i,it;
        float a;
        Vector3 p[6] = {
            Vector3::Create(0,0,1),
            Vector3::Create(0,0,-1),
            Vector3::Create(-1,-1,0),
            Vector3::Create(1,-1,0),
            Vector3::Create(1,1,0),
            Vector3::Create(-1,1,0)
        };

        Vector3 pa,pb,pc;
        int nt = 0,ntold;
        
        /* Create the level 0 object */
        a = 1.0f / sqrtf(2.0f);
        for (i=0;i<6;i++) {
            p[i].x *= a;
            p[i].y *= a;
        }
        f[0].p1 = p[0]; f[0].p2 = p[3]; f[0].p3 = p[4];
        f[1].p1 = p[0]; f[1].p2 = p[4]; f[1].p3 = p[5];
        f[2].p1 = p[0]; f[2].p2 = p[5]; f[2].p3 = p[2];
        f[3].p1 = p[0]; f[3].p2 = p[2]; f[3].p3 = p[3];
        f[4].p1 = p[1]; f[4].p2 = p[4]; f[4].p3 = p[3];
        f[5].p1 = p[1]; f[5].p2 = p[5]; f[5].p3 = p[4];
        f[6].p1 = p[1]; f[6].p2 = p[2]; f[6].p3 = p[5];
        f[7].p1 = p[1]; f[7].p2 = p[3]; f[7].p3 = p[2];
        nt = 8;
        
        /* Bisect each edge and move to the surface of a unit sphere */
        for (it=0;it<iterations;it++) {
            ntold = nt;
            for (i=0;i<ntold;i++) {
                pa.x = (f[i].p1.x + f[i].p2.x) / 2;
                pa.y = (f[i].p1.y + f[i].p2.y) / 2;
                pa.z = (f[i].p1.z + f[i].p2.z) / 2;
                pb.x = (f[i].p2.x + f[i].p3.x) / 2;
                pb.y = (f[i].p2.y + f[i].p3.y) / 2;
                pb.z = (f[i].p2.z + f[i].p3.z) / 2;
                pc.x = (f[i].p3.x + f[i].p1.x) / 2;
                pc.y = (f[i].p3.y + f[i].p1.y) / 2;
                pc.z = (f[i].p3.z + f[i].p1.z) / 2;
                pa.normalize();
                pb.normalize();
                pc.normalize();
                f[nt].p1 = f[i].p1; f[nt].p2 = pa; f[nt].p3 = pc; nt++;
                f[nt].p1 = pa; f[nt].p2 = f[i].p2; f[nt].p3 = pb; nt++;
                f[nt].p1 = pb; f[nt].p2 = f[i].p3; f[nt].p3 = pc; nt++;
                f[i].p1 = pa;
                f[i].p2 = pb;
                f[i].p3 = pc;
            }
        }
        
        m_sphereVertices = (GLfloat *)malloc(sizeof(GLfloat) * m_cVertices * 3);
        assert(m_sphereVertices != NULL);
        GLfloat *pDest = m_sphereVertices;
        for(int facet_index=0; facet_index < facet_count; facet_index++) {
            *pDest++ = f[facet_index].p1.x;
            *pDest++ = f[facet_index].p1.y;
            *pDest++ = f[facet_index].p1.z;
            *pDest++ = f[facet_index].p2.x;
            *pDest++ = f[facet_index].p2.y;
            *pDest++ = f[facet_index].p2.z;
            *pDest++ = f[facet_index].p3.x;
            *pDest++ = f[facet_index].p3.y;
            *pDest++ = f[facet_index].p3.z;
        }
    }
}
