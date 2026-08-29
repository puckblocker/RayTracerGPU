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

    Node *root = nullptr;

    // NODE FUNCTIONS
    void deleteNode();
    void deleteTree(Node*);

    // BVH FUNCTIONS
    void buildBVH(std::vector<Intersect::Triangle> &);
    Node *workerBVH(std::vector<Intersect::Triangle> &, int, int);
};