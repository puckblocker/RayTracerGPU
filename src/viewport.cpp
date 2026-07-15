// ========================================
// Purpose: CAMERA / VIEWPORT CODE
// ========================================

#include "viewport.h"

// ========================================
// VIEWPORT CODE
// ========================================
void Camera::camViewUpdate()
{
    // BASIS VARIABLES
    glm::vec3 normGaze = glm::normalize(gaze);
    glm::vec3 normUp = glm::normalize(up);                            // normalized up vector to help find the 3rd basis vector
    glm::vec3 orthoU = glm::normalize(glm::cross(normGaze, normUp));  // 3rd vector for cartesian that is perpendicular to the other vectors
    glm::vec3 orthoUp = glm::normalize(glm::cross(orthoU, normGaze)); // fix the up vector to be orthogonal to other vectors
    basis.xhat = orthoU;
    basis.yhat = orthoUp;
    basis.zhat = -normGaze; // standard right hand system is reason for negative (flips variable directions without it)

    // VIEWPORT CALCULATION
    glm::vec3 viewportCenter = origin + (length * gaze);
    viewport.origin = viewportCenter - 0.5f * (viewport.width * orthoU) - 0.5f * (viewport.height * orthoUp);

    // LENS DISTANCE CALCULATION
    focalDist = 0.5f * focusDist;

    // PIXEL CALCULATIONS
    image.pixelHeight = viewport.height / image.height;
    image.pixelWidth = viewport.width / image.width;
}