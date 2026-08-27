// ========================================
// Purpose: BOUNDING VOLUME SETUP BLUEPRINT
// ========================================
#pragma once
#include "package_manager.h"
#include "intersection.h"

class BVH
{
public:
    int n; // number of nodes

    // ----------------------------------------
    // BOUNDING BOXES SETUP
    // ----------------------------------------
    struct BoundBox
    {
        std::vector<Intersect::Triangle> trianglePrim; // all triangle primitives held within a bounding box
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
    Node *newNode();
    Node *insertNode();
    Node *removeNode();

    void boundHierarchy();
};