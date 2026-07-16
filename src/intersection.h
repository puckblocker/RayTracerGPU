// ========================================
// PURPOSE: INTERSECTION BLUEPRINT
// ========================================

#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "rayData.h"

// Info On Intersection
struct HitInfo
{
    // ========================================
    // HIT INFO BLUEPRINT
    // ========================================
    glm::vec3 point;
    float distance;

    glm::vec3 normal; // normal to surface at intersect point
    unsigned int objID;

    int valid;

    // ========================================
    // MATERIAL BLUEPRINT
    // ========================================
    struct Material
    {
        glm::vec3 albedo; // object's reflectance/color
        float roughness;  // 0 = smooth // 1 = rough

        float metallic; // metallic / conductor
        float ior;      // Index of Refraction (is object glass)
        int emissive;   // is object a light
        float z;        // depth
        float layerIOR; // ior for layered
    };

    Material mat;
};

// ========================================
// INTERSECTION BLUEPRINT
// ========================================
namespace Intersect
{
    // ========================================
    // TRANSFORM BLUEPRINT
    // ========================================
    struct xForm
    {
        glm::mat4 transform; // matrix transforms
        int crntID;
        int prntID;
        float padding1 = 0.0f;
        float padding2 = 0.0f;
    };

    // ========================================
    // LOCAL SPACE BLUEPRINT
    // ========================================
    struct LocalSpaceData
    {
        Ray localRay;
        glm::mat4 wrldToLocal;
        glm::mat3 nrmMtrx;
    };

    // ========================================
    // SPHERE BLUEPRINT
    // ========================================
    struct Sphere
    {
        glm::vec3 albedo;
        unsigned int objID;
        float roughness;
        float metallic;
        float ior;
        float emissive;
        float z;        // depth
        float layerIOR; // ior for layered
        int animated = false;
        float padding1 = 0.0f;

        glm::vec3 center;
        float radius;
    };

    // ========================================
    // TRIANGLE BLUEPRINT
    // ========================================
    struct Triangle
    {
        glm::vec3 albedo;
        unsigned int objID;
        float roughness;
        float metallic;
        float ior;
        float emissive;
        float z;        // depth
        float layerIOR; // ior for layered
        int animated = false;
        float padding1 = 0.0f;

        glm::vec3 p0;
        float padding1 = 0.0f;

        glm::vec3 p1;
        float padding2 = 0.0f;

        glm::vec3 p2;
        float padding3 = 0.0f;
    };

    // ========================================
    // PLANE BLUEPRINT
    // ========================================
    struct Plane
    {
        glm::vec3 albedo;
        unsigned int objID;
        float roughness;
        float metallic;
        float ior;
        float emissive;
        float z;        // depth
        float layerIOR; // ior for layered
        int animated = false;
        float padding1 = 0.0f;

        glm::vec3 position;
        float padding1 = 0.0f;

        glm::vec3 normal;
        float padding2 = 0.0f;
    };
};