// ========================================
// PURPOSE: RAY BLUEPRINT
// ========================================
#pragma once // Ensures header file only include once throughout program

#include <glm/glm.hpp>

class Ray
{
public:
    glm::vec3 origin;
    float padding1 = 0.0f;
    glm::vec3 direction;
    double time;
};