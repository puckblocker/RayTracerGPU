// ========================================
// Purpose: PACKAGE MANAGER BLUEPRINT
// ========================================

#include "package_manager.h"
#include <iostream>
#include <fstream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace Intersect;

// ========================================
// PACKAGER
// ========================================
Packager::Package Packager::packager()
{
    // INSTANTIATE STRUCTS
    Package newPackage;
    Ray ray;

    // SCENE READER
    loadScene("scene.txt", newPackage);

    // CAMERA SETUP
    newPackage.camera.camViewUpdate();

    // PACKAGE & SEND DATA
    return newPackage;
}

// ========================================
// SCENE LOADER
// ========================================
void Packager::loadScene(const std::string &filename, Package &newPackage)
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

            newPackage.spheres.push_back(newSphere);

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
            newPackage.triangles.push_back(newTriangle);
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

            newPackage.planes.push_back(newPlane);
        }

        // ========================================
        // POINT LIGHT INFO
        // ========================================
        else if (type == "pLight")
        {
            Light::pLight newPointLight;
            file >> newPointLight.origin.x >> newPointLight.origin.y >> newPointLight.origin.z;
            file >> newPointLight.color.r >> newPointLight.color.g >> newPointLight.color.b;

            newPackage.pointLights.push_back(newPointLight);
        }

        // ========================================
        // AREA LIGHT INFO
        // ========================================
        else if (type == "aLight")
        {
            Light::aLight newAreaLight;
            file >> newAreaLight.origin.x >> newAreaLight.origin.y >> newAreaLight.origin.z;
            file >> newAreaLight.color.r >> newAreaLight.color.g >> newAreaLight.color.b;
            file >> newAreaLight.normal.x >> newAreaLight.normal.y >> newAreaLight.normal.z;
            file >> newAreaLight.u.x >> newAreaLight.u.y >> newAreaLight.u.z;
            file >> newAreaLight.v.x >> newAreaLight.v.y >> newAreaLight.v.z;

            newPackage.areaLights.push_back(newAreaLight);
        }

        // ========================================
        // DIRECTIONAL LIGHT INFO
        // ========================================
        else if (type == "dLight")
        {
            Light::dLight newDirLight;
            file >> newDirLight.direction.x >> newDirLight.direction.y >> newDirLight.direction.z;
            file >> newDirLight.color.r >> newDirLight.color.g >> newDirLight.color.b;

            newPackage.directionalLights.push_back(newDirLight);
        }

        // ========================================
        // CAMERA INFO
        // ========================================
        else if (type == "Camera")
        {
            file >> newPackage.camera.origin.x >> newPackage.camera.origin.y >> newPackage.camera.origin.z;
            file >> newPackage.camera.up.x >> newPackage.camera.up.y >> newPackage.camera.up.z;
            file >> newPackage.camera.gaze.x >> newPackage.camera.gaze.y >> newPackage.camera.gaze.z;
            file >> newPackage.camera.length;
        }

        // ========================================
        // VIEWPORT INFO
        // ========================================
        else if (type == "Viewport")
        {
            file >> newPackage.camera.viewport.height >> newPackage.camera.viewport.width;
        }

        // ========================================
        // LENS INFO
        // ========================================
        else if (type == "Lens")
        {
            file >> newPackage.camera.lensDiameter >> newPackage.camera.focusDist;
            std::cout << "Lens diameter: " << newPackage.camera.lensDiameter << std::endl;
        }

        // ========================================
        // TRANSFORM INFO
        // ========================================
        else if (type == "Transform")
        {
            // VARIABLES
            Intersect::xForm trnsfrm;
            trnsfrm.transform = glm::mat4(1.0f);
            glm::vec3 rot(1.0f);
            glm::vec3 trns(1.0f);
            float scale = 1.0f;

            // OBJECT IDs
            file >> trnsfrm.crntID >> trnsfrm.prntID;
            // ROTATION
            file >> rot.x >> rot.y >> rot.z;
            // TRANSLATION
            file >> trns.x >> trns.y >> trns.z;
            // SCALE
            file >> scale;

            // TRANSFORMS
            trnsfrm.transform = glm::translate(trnsfrm.transform, trns);
            // NaN CHECK FROM ROTATION (prevent division by zero)
            if (glm::length(glm::vec3(rot)) > 0.0001f)
            {
                trnsfrm.transform = glm::rotate(trnsfrm.transform, glm::radians(rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
                trnsfrm.transform = glm::rotate(trnsfrm.transform, glm::radians(rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
                trnsfrm.transform = glm::rotate(trnsfrm.transform, glm::radians(rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
            }
            trnsfrm.transform = glm::scale(trnsfrm.transform, glm::vec3(scale));

            newPackage.xForms.push_back(trnsfrm);

            std::cout << "Transform for ID " << trnsfrm.crntID << ":\n";
            std::cout << glm::to_string(trnsfrm.transform) << "\n\n";
        }
    }
    file.close();
}