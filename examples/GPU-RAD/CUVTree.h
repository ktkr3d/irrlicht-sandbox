#ifndef CUVTREE_H
#define CUVTREE_H

#include "types.h"

struct sectorNode
{
    core::array<core::triangle3df> triangles;
    core::array<core::triangle3df> triangles3D;

    core::aabbox3df dimenBox;
    core::aabbox3df testBox;

    sectorNode* parent;

    bool isLeaf;
    sectorNode* nodes;
};

class CUVTree
{
public:
    CUVTree(const core::array<topoData>& topoMap);
    ~CUVTree();
    sectorNode* createRootNode(const core::array<topoData>& topoMap);
    void split(sectorNode* inSector);
    topoData getTriangle(const core::vector3df point);


private:
void relaxBoundries();
sectorNode* root;
core::array<sectorNode*> leaflist;
core::array<sectorNode*> nlist;
};

#endif // CUVTREE_H
