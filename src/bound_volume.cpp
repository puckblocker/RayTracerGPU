// ========================================
// Purpose: BOUNDING VOLUME SETUP CODE
// ========================================
#include "bound_volume.h"

// ========================================
// BOUNDING VOLUME HIERARCHY CREATION
// ========================================
void BVH::boundHierarchy(std::vector<Intersect::Triangle> primArray) // pass in vector of all primitives
{
    n = primArray.size();
    nodeTotal = 2 * n - 1;

    // TEMP REMOVE
    int startIndex = 0;
    int endIndex = 10;

    // BOX MIN AND MAX
    glm::vec3 boxMin = glm::vec3(-std::numeric_limits<float>::infinity()); // close to infinity
    glm::vec3 boxMax = glm::vec3(std::numeric_limits<float>::infinity());

    // ----------------------------------------
    // MIDPOINT SPLIT BVH CREATION
    // ----------------------------------------

    // FIND LONGEST BOX SIZE
    for (int i = startIndex; i < endIndex; i++)
    {
        Intersect::Triangle &prim = primArray[i];

        // GRAB PRIMITVE MIN AND MAX SIZE
        glm::vec3 primMin = glm::min(prim.p0, glm::min(prim.p1, prim.p2));
        glm::vec3 primMax = glm::max(prim.p0, glm::max(prim.p1, prim.p2));

        // COMPARE AXISES
        boxMin = glm::min(boxMin, primMin);
        boxMax = glm::max(boxMax, primMax);
    }

    // FIND LONGEST AXIS
    glm::vec3 boxSize = boxMax - boxMin;
    int longAxis = 0; // x = 0, y = 1, z = 2

    if (boxSize.y > boxSize.x && boxSize.y > boxSize.z)
        longAxis = 1;
    else if (boxSize.z > boxSize.x && boxSize.z > boxSize.y)
        longAxis = 2;

    // SORT PRIMITIVES VIA MIDPOINT
    float center; // center point of primitive
    for (int i = 0; i < n; i++)
    {
        Intersect::Triangle &prim = primArray[i];
        switch (longAxis)
        {
        // X AXIS
        case (0):
            center = (prim.p0.x + prim.p1.x + prim.p2.x) / 3;
            if (center <= boxSize.x / 2)
            {
            }
        // Y AXIS
        case (1):
            center = (prim.p0.y + prim.p1.y + prim.p2.y) / 3;
            if (center <= boxSize.y / 2)
            {
            }
        // Z AXIS
        case (2):
            center = (prim.p0.z + prim.p1.z + prim.p2.z) / 3;
            if (center <= boxSize.z / 2)
            {
            }
        }
    }
}

// ========================================
// BST CREATION
// ========================================
BVH::Node *BVH::createNode(Intersect::Triangle triangle)
{
}

BVH::Node *BVH::insertNode(Intersect::Triangle triangle)
{
}

BVH::Node *BVH::removeNode()
{
}