// ========================================
// Purpose: CAMERA & VIEWPORT BLUEPRINT
// ========================================

#pragma once // Ensures header file only include once throughout program

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include "rayData.h"

class Camera
{
public:
    // ========================================
    // CAMERA BLUEPRINT
    // ========================================
    glm::vec3 origin = glm::vec3(0.0f); // focal point, standard right hand coords
    glm::vec3 up = glm::vec3(0.0f);     // vertical orientation
    glm::vec3 gaze = glm::vec3(0.0f);   // direction camera is facing
    float length;                       // distance from viewport center to camera
    float lensDiameter;                 // size of camera lens (user provided)
    float focusDist;                    // camera focus distance (user provided)
    float focalDist;                    // camera focal distance

    // ========================================
    // VIEWPORT BLUEPRINT
    // ========================================
    struct Viewport
    {
        float height; // values so view is simplified to between -1 and +1
        float width;
        glm::vec3 origin;
    };

    // ========================================
    // IMAGE BLUEPRINT
    // ========================================
    struct Image
    {
        int width = 640;
        int height = 640;
        float pixelHeight; // individual pixel height
        float pixelWidth;
    };

    // ========================================
    // CAMERA BASIS BLUEPRINT
    // ========================================
    struct Basis
    {
        glm::vec3 xhat; // right
        glm::vec3 yhat; // up
        glm::vec3 zhat; // forward
    };

    // ========================================
    // COMPRESSED CAMERA BLUEPRINT
    // ========================================
    struct CompCam
    {
        // CAMERA STATS (Needed for GPU Side)
        glm::vec3 camOrigin;
        float padding1 = 0.0f; // padding to make 16 byt chunk (GPU prefers)

        glm::vec3 xhat;
        float padding2 = 0.0f;

        glm::vec3 yhat;
        float focusDist;

        glm::vec3 origin;
        float lensDiameter;
    };

    // STRUCT INSTANTIATES
    Viewport viewport;
    Image image;
    Basis basis;
    CompCam compCam;

    // FUNCTION SIGNATURES
    void camViewUpdate();
};