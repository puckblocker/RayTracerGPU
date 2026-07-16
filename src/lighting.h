// ========================================
// Purpose: LIGHTING BLUEPRINT
// ========================================

#pragma once

#include <glm/glm.hpp>

#include "intersection.h"
#include "config.h"
#include "viewport.h"
#include "rayData.h"

class Light
{
public:
    // ========================================
    //  POINT LIGHT BLUEPRINT
    // ========================================
    struct pLight
    {
        glm::vec3 origin = glm::vec3(0.0f); // light position in global coords
        float padding1 = 0.0f;

        glm::vec3 color = glm::vec3(0.0f);  // color / power
        float padding2 = 0.0f;
    };

    // ========================================
    //  DIRECTIONAL LIGHT BLUEPRINT
    // ========================================
    struct dLight
    {
        glm::vec3 direction = glm::vec3(0.0f);
        float padding1 = 0.0f;

        glm::vec3 color = glm::vec3(0.0f);
        float padding2 = 0.0f;
    };

    // ========================================
    //  AREA LIGHT BLUEPRINT
    // ========================================
    struct aLight
    {
        glm::vec3 origin = glm::vec3(0.0f);
        float padding1 = 0.0f;

        glm::vec3 color = glm::vec3(0.0f);
        float padding2 = 0.0f;

        glm::vec3 normal;
        float padding3 = 0.0f;

        glm::vec3 u; // width & rotation
        float padding4 = 0.0f;
        
        glm::vec3 v; // height & rotation
        float padding5 = 0.0f;
    };
};