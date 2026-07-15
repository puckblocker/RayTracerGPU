// ========================================
// Purpose: PACKAGE MANAGER BLUEPRINT
// ========================================

#include "package_manager.h"
#include <iostream>
#include <fstream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "omp.h"

using namespace Intersect;

// ========================================
// PACKAGER
// ========================================
std::vector<Packager::package> Packager::packager()
{
    package::camera.camViewUpdate();

    Ray ray;
}

// ========================================
// SCENE LOADER
// ========================================
void Packager::loadScene(const std::string &filename)
{
    std::ifstream file(filename);

    // FILE CHECKER
    if (!file.is_open())
    {
        std::cout << "Failed to Open Scene File: " << filename << std::endl;
        return;
    }

    // VARIABLES
    std::string type;
    unsigned int objID = 0;

    // FILE READER
    while (file >> type)
    {
        // ========================================
        // SPHERE INFO
        // ========================================
        if (type == "Sphere")
        {
            // Shape Stats
            Intersect::Sphere newSphere;
            file >> newSphere.center.x >> newSphere.center.y >> newSphere.center.z;
            file >> newSphere.radius;
            // Material Stats
            file >> newSphere.albedo.r >> newSphere.albedo.g >> newSphere.albedo.b;
            file >> newSphere.metallic >> newSphere.roughness >> newSphere.ior >> newSphere.emissive >> newSphere.z >> newSphere.layerIOR;
            file >> newSphere.animated;
            objID += 1;
            newSphere.objID = objID;

            spheres.push_back(newSphere);

            std::cout << "Sphere Position: " << newSphere.center.x << " " << newSphere.center.y << " " << newSphere.center.z << std::endl;
            std::cout << "Radius: " << newSphere.radius << std::endl;
            std::cout << "Metallic: " << newSphere.metallic << " Roughness: " << newSphere.roughness << " IOR: " << newSphere.ior << " Emissive: " << newSphere.emissive << std::endl;
        }

        // ========================================
        // TRIANGLE INFO
        // ========================================
        else if (type == "Triangle")
        {
            Intersect::Triangle newTriangle;
            // SHAPE STATS
            file >> newTriangle.p0.x >> newTriangle.p0.y >> newTriangle.p0.z;
            file >> newTriangle.p1.x >> newTriangle.p1.y >> newTriangle.p1.z;
            file >> newTriangle.p2.x >> newTriangle.p2.y >> newTriangle.p2.z;
            // MATERIAL STATS
            file >> newTriangle.albedo.r >> newTriangle.albedo.g >> newTriangle.albedo.b;
            file >> newTriangle.metallic >> newTriangle.roughness >> newTriangle.ior >> newTriangle.emissive >> newTriangle.z >> newTriangle.layerIOR;

            objID += 1;
            newTriangle.objID = objID;
            triangles.push_back(newTriangle);
        }

        // ========================================
        // PLANE INFO
        // ========================================
        else if (type == "Plane")
        {
            Intersect::Plane newPlane;
            // SHAPE STATS
            file >> newPlane.position.x >> newPlane.position.y >> newPlane.position.z;
            file >> newPlane.normal.x >> newPlane.normal.y >> newPlane.normal.z;
            // MATERIAL STATS
            file >> newPlane.albedo.r >> newPlane.albedo.g >> newPlane.albedo.b;
            file >> newPlane.metallic >> newPlane.roughness >> newPlane.ior >> newPlane.emissive >> newPlane.z >> newPlane.layerIOR;
            objID += 1;
            newPlane.objID = objID;

            planes.push_back(newPlane);
        }

        // ========================================
        // POINT LIGHT INFO
        // ========================================
        else if (type == "pLight")
        {
            file >> pointLight.origin.x >> pointLight.origin.y >> pointLight.origin.z;
            file >> pointLight.color.r >> pointLight.color.g >> pointLight.color.b;
        }

        // ========================================
        // AREA LIGHT INFO
        // ========================================
        else if (type == "aLight")
        {
            file >> areaLight.origin.x >> areaLight.origin.y >> areaLight.origin.z;
            file >> areaLight.color.r >> areaLight.color.g >> areaLight.color.b;
            file >> areaLight.normal.x >> areaLight.normal.y >> areaLight.normal.z;
            file >> areaLight.u.x >> areaLight.u.y >> areaLight.u.z;
            file >> areaLight.v.x >> areaLight.v.y >> areaLight.v.z;
        }

        // ========================================
        // DIRECTIONAL LIGHT INFO
        // ========================================
        else if (type == "dLight")
        {
            file >> directionalLight.direction.x >> directionalLight.direction.y >> directionalLight.direction.z;
            file >> directionalLight.color.r >> directionalLight.color.g >> directionalLight.color.b;
        }

        // ========================================
        // CAMERA INFO
        // ========================================
        else if (type == "Camera")
        {
            file >> camera.origin.x >> camera.origin.y >> camera.origin.z;
            file >> camera.up.x >> camera.up.y >> camera.up.z;
            file >> camera.gaze.x >> camera.gaze.y >> camera.gaze.z;
            file >> camera.length;
        }

        // ========================================
        // VIEWPORT INFO
        // ========================================
        else if (type == "Viewport")
        {
            file >> camera.viewport.height >> camera.viewport.width;
        }

        // ========================================
        // LENS INFO
        // ========================================
        else if (type == "Lens")
        {
            file >> camera.lensDiameter >> camera.focusDist;
            std::cout << "Lens diameter: " << camera.lensDiameter << std::endl;
        }

        // ========================================
        // TRANSFORM INFO
        // ========================================
        else if (type == "Transform")
        {
            // VARIABLES
            Intersect::xForm trnsfrm;
            trnsfrm.transform = glm::mat4(1.0f);
            glm::vec4 rot(1.0f);
            glm::vec3 trns(1.0f);
            float scale = 1.0f;

            // OBJECT IDs
            file >> trnsfrm.crntID >> trnsfrm.prntID;
            // ROTATION
            file >> rot.x >> rot.y >> rot.z >> rot.w;
            // TRANSLATION
            file >> trns.x >> trns.y >> trns.z;
            // SCALE
            file >> scale;

            // TRANSFORMS
            trnsfrm.transform = glm::translate(trnsfrm.transform, trns);
            trnsfrm.transform = glm::rotate(trnsfrm.transform, glm::radians(rot.w), glm::vec3(rot));
            trnsfrm.transform = glm::scale(trnsfrm.transform, glm::vec3(scale));

            xForms.push_back(trnsfrm);

            std::cout << "Transform for ID " << trnsfrm.crntID << ":\n";
            std::cout << glm::to_string(trnsfrm.transform) << "\n\n";
        }
    }
    file.close();
}