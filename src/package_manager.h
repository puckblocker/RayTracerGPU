// ========================================
// Purpose: PACKAGE MANAGER BLUEPRINT
// ========================================

#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "rayData.h"
#include "intersection.h"
#include "lighting.h"
#include "viewport.h"

class Packager
{
public:
    // ========================================
    // PACKAGE MANAGER BLUEPRINT
    // ========================================
    struct Package
    {
        // VARIABLES
        Light light;
        Camera camera;

        // SCENE OBJECTS
        std::vector<Intersect::Sphere> spheres;
        std::vector<Intersect::Plane> planes;
        std::vector<Intersect::Triangle> triangles;
        std::vector<Intersect::xForm> xForms;
        Light::pLight pointLight;
        Light::dLight directionalLight;
        Light::aLight areaLight;
    };

    // FUNCTION SIGNATURES
    Package packager();
    void loadScene(const std::string &filename, Package &newPackage);
};