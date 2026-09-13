// ========================================
// Purpose: BOUNDING VOLUME SETUP BLUEPRINT
// ========================================
#pragma once

#include "intersection.h"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

class BVH
{
public:
    int n; // number of primtives
    int nodeTotal;

    // ----------------------------------------
    // BOUNDING BOXES SETUP
    // ----------------------------------------
    struct BoundBox
    {
        int startIndex; // starting point for primtive array
        int endIndex;   // ending point for primitive array
        int rightIndex; // right child index (for GPU traversal)

        glm::vec3 boxMin;
        float padding1 = 0.0;

        glm::vec3 boxMax;
        float padding2 = 0.0;
    };

    // ----------------------------------------
    // BST SETUP (TRIANGLE)
    // ----------------------------------------
    struct Node
    {
        BoundBox boundBox;
        Node *left;
        Node *right;
    };

    // NODE FUNCTIONS
    void deleteTree(Node *);

    // BVH FUNCTIONS
    std::vector<BoundBox> buildBVH(std::vector<Intersect::Triangle> &, std::vector<glm::vec3> &);
    Node *workerBVH(std::vector<Intersect::Triangle> &, std::vector<glm::vec3> &, int, int, glm::vec3, glm::vec3);
    void traverseBVH(Node *, std::vector<BoundBox> &);
};