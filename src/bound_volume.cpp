// ========================================
// Purpose: BOUNDING VOLUME SETUP CODE
// ========================================
#include "bound_volume.h"

// ========================================
// BVH MANAGER
// ========================================
void BVH::buildBVH(std::vector<Intersect::Triangle> &masterArray)
{
    // BUILD BVH TREE
    root = workerBVH(masterArray, 0, masterArray.size());
}

// ========================================
// BVH RECURSIVE WORKER
// ========================================
BVH::Node *BVH::workerBVH(std::vector<Intersect::Triangle> &primArray, int startIndex, int endIndex) // pass in vector of all primitives
{
    // SETUP
    Node *newNode = new Node(); // allocate memory for new node
    n = primArray.size();
    nodeTotal = 2 * n - 1;

    // CREATE BASE NODE
    if (endIndex - startIndex == n)
    {
        newNode->boundBox.startIndex = 0;
        newNode->boundBox.endIndex = n;
    }

    // CHECK FOR LEAF NODE
    if (endIndex - startIndex <= 1)
    {
        newNode->left = nullptr;
        newNode->right = nullptr;
        newNode->boundBox.startIndex = startIndex;
        newNode->boundBox.endIndex = endIndex;

        return newNode;
    }

    // BOX MIN AND MAX
    glm::vec3 boxMin = glm::vec3(std::numeric_limits<float>::infinity()); // close to infinity
    glm::vec3 boxMax = glm::vec3(-std::numeric_limits<float>::infinity());

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
    int leftIndxStart = startIndex;
    int leftIndxEnd = startIndex;
    int rightIndxStart, rightIndxEnd;

    for (int i = startIndex; i < endIndex; i++)
    {
        Intersect::Triangle &prim = primArray[i];
        switch (longAxis)
        {
        // X AXIS
        case (0):
            center = (prim.p0.x + prim.p1.x + prim.p2.x) / 3;
            if (center <= boxMin.x + (boxSize.x / 2.0f))
            {
                // SWAP VECTORS
                std::swap(primArray[i], primArray[leftIndxEnd]); // sort array via mid point
                leftIndxEnd++;                                   // increment for the swap
            }
        // Y AXIS
        case (1):
            center = (prim.p0.y + prim.p1.y + prim.p2.y) / 3;
            if (center <= boxMin.y + (boxSize.y / 2.0f))
            {
                // SWAP VECTORS
                std::swap(primArray[i], primArray[leftIndxEnd]);
                leftIndxEnd++;
            }
        // Z AXIS
        case (2):
            center = (prim.p0.z + prim.p1.z + prim.p2.z) / 3;
            if (center <= boxMin.z + (boxSize.z / 2.0f))
            {
                // SWAP VECTORS
                std::swap(primArray[i], primArray[leftIndxEnd]);
                leftIndxEnd++;
            }
        }
    }

    // RIGHT NODE INDEX SETUP
    rightIndxStart = leftIndxEnd;
    rightIndxEnd = endIndex;

    // ----------------------------------------
    // CREATE BOUNDING BOXES
    // ----------------------------------------

    // node->left = createNode(primArray, node, leftIndxStart, leftIndxEnd, true);
    newNode->left = workerBVH(primArray, leftIndxStart, leftIndxEnd);
    newNode->right = workerBVH(primArray, rightIndxStart, rightIndxEnd);

    return newNode;
}

// ========================================
// DELETE BVH TREE
// ========================================
void BVH::deleteTree(Node *node)
{
    // CHECK FOR END OF TREE
    if (node == nullptr)
    {
        return;
    }

    // DESCEND TREE
    deleteTree(node->left);
    deleteTree(node->right);

    // DELETE NODE
    delete node;
}

// ========================================
// DELETE NODE
// ========================================
void BVH::deleteNode()
{
}
