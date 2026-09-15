// ========================================
// Purpose: BOUNDING VOLUME SETUP CODE
// ========================================
#include "bound_volume.h"

// ========================================
// BVH MANAGER
// ========================================
std::vector<BVH::BoundBox> BVH::buildBVH(std::vector<Intersect::Triangle> &masterArray, std::vector<glm::vec3> &vertBuffer)
{
    std::vector<BoundBox> boxArray; // compressed BVH
    Node *root = nullptr;

    // BUILD BVH TREE
    root = workerBVH(masterArray, vertBuffer, 0, masterArray.size(), glm::vec3(0.0), glm::vec3(0.0));

    // COMPRESS INTO LINEAR ARRAY
    traverseBVH(root, boxArray);

    // DELETE TREE TO CLEAR MEMORY
    deleteTree(root);

    // RETURN COMPRESSED BVH
    return boxArray;
}

// ========================================
// BVH RECURSIVE WORKER
// ========================================
BVH::Node *BVH::workerBVH(std::vector<Intersect::Triangle> &primArray, std::vector<glm::vec3> &vertBuffer, int startIndex, int endIndex, glm::vec3 boxMin, glm::vec3 boxMax) // pass in vector of all primitives
{
    // SETUP
    Node *newNode = new Node(); // allocate memory for new node
    n = primArray.size();

    // CREATE BASE NODE
    if (endIndex - startIndex == n)
    {
        newNode->boundBox.startIndex = 0;
        newNode->boundBox.endIndex = n;
        newNode->boundBox.boxMin = glm::vec3(0.0);
        newNode->boundBox.boxMax = glm::vec3(0.0);
    }

    // CHECK FOR LEAF NODE
    if (endIndex - startIndex <= 1)
    {
        newNode->left = nullptr;
        newNode->right = nullptr;
        newNode->boundBox.startIndex = startIndex;
        newNode->boundBox.endIndex = endIndex;
        newNode->boundBox.boxMin = boxMin;
        newNode->boundBox.boxMax = boxMax;

        nodeTotal++;
        return newNode;
    }

    // BOX MIN AND MAX
    boxMin = glm::vec3(std::numeric_limits<float>::infinity()); // close to infinity
    boxMax = glm::vec3(-std::numeric_limits<float>::infinity());

    // ----------------------------------------
    // MIDPOINT SPLIT BVH CREATION
    // ----------------------------------------

    // FIND LONGEST BOX SIZE
    for (int i = startIndex; i < endIndex; i++)
    {
        Intersect::Triangle &prim = primArray[i];
        glm::vec3 p0, p1, p2;

        // TRIANGLE CONSTRUCTION
        if (prim.faces.x >= 0.0) // check for faces
        {
            p0 = vertBuffer[int(prim.faces.x) - 1];
            p1 = vertBuffer[int(prim.faces.y) - 1];
            p2 = vertBuffer[int(prim.faces.z) - 1];
        }
        else
        {
            p0 = prim.p0;
            p1 = prim.p1;
            p2 = prim.p2;
        }

        // GRAB PRIMITVE MIN AND MAX SIZE
        glm::vec3 primMin = glm::min(p0, glm::min(p1, p2));
        glm::vec3 primMax = glm::max(p0, glm::max(p1, p2));

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
        glm::vec3 p0, p1, p2;

        // TRIANGLE CONSTRUCTION
        if (prim.faces.x >= 0.0) // check for faces
        {
            p0 = vertBuffer[int(prim.faces.x) - 1];
            p1 = vertBuffer[int(prim.faces.y) - 1];
            p2 = vertBuffer[int(prim.faces.z) - 1];
        }
        else
        {
            p0 = prim.p0;
            p1 = prim.p1;
            p2 = prim.p2;
        }

        switch (longAxis)
        {
        // X AXIS
        case (0):
            center = (p0.x + p1.x + p2.x) / 3;
            if (center <= boxMin.x + (boxSize.x / 2.0f))
            {
                // SWAP VECTORS
                std::swap(primArray[i], primArray[leftIndxEnd]); // sort array via mid point
                leftIndxEnd++;                                   // increment for the swap
            }
            break;
        // Y AXIS
        case (1):
            center = (p0.y + p1.y + p2.y) / 3;
            if (center <= boxMin.y + (boxSize.y / 2.0f))
            {
                // SWAP VECTORS
                std::swap(primArray[i], primArray[leftIndxEnd]);
                leftIndxEnd++;
            }
            break;
        // Z AXIS
        case (2):
            center = (p0.z + p1.z + p2.z) / 3;
            if (center <= boxMin.z + (boxSize.z / 2.0f))
            {
                // SWAP VECTORS
                std::swap(primArray[i], primArray[leftIndxEnd]);
                leftIndxEnd++;
            }
            break;
        }
    }

    // RIGHT NODE INDEX SETUP
    rightIndxStart = leftIndxEnd;
    rightIndxEnd = endIndex;

    // ----------------------------------------
    // CREATE BOUNDING BOXES
    // ----------------------------------------

    // node->left = createNode(primArray, node, leftIndxStart, leftIndxEnd, true);
    newNode->left = workerBVH(primArray, vertBuffer, leftIndxStart, leftIndxEnd, boxMin, boxMax);
    newNode->right = workerBVH(primArray, vertBuffer, rightIndxStart, rightIndxEnd, boxMin, boxMax);

    return newNode;
}

// ========================================
// COMPRESS DFS TRAVERSAL BVH
// ========================================
void BVH::traverseBVH(Node *node, std::vector<BoundBox> &boxArray)
{
    // CHECK FOR END OF TREE
    if (node == nullptr)
    {
        return;
    }

    // ----------------------------------------
    // COMPRESS DFS TRAVERSAL BVH
    // ----------------------------------------
    int crntIndx = boxArray.size();

    boxArray.push_back(node->boundBox); // store parent node
    traverseBVH(node->left, boxArray);  // step to left child

    if (node->right != nullptr)
    {
        boxArray[crntIndx].rightIndex = boxArray.size();
        traverseBVH(node->right, boxArray); // step to right child
    }

    return;
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