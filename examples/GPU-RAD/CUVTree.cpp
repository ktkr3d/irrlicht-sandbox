#include "CUVTree.h"

CUVTree::CUVTree(const core::array<topoData>& topoMap)
{
    createRootNode(topoMap);
    split(root);

    relaxBoundries();
}

CUVTree::~CUVTree()
{
    //delete all the "new"'ed sectors
    for(int i=0; i<nlist.size(); i++)
    {
        sectorNode* n= nlist[i];
        if(!n->isLeaf)
        {
            delete [] n->nodes;
        }
    }
    delete root;
}

sectorNode* CUVTree::createRootNode(const core::array<topoData>& topoMap)
{
    sectorNode* node= new sectorNode();

    for(int i=0; i<topoMap.size(); i++)
    {
        node->triangles.push_back(topoMap[i].tri2D);
        node->triangles3D.push_back(topoMap[i].tri3D);
    }

    node->isLeaf=false;
    node->parent=0;
    node->dimenBox=core::aabbox3df(0.0f,0.0f,-1.0f,1.0f,1.0f,1.0f);
    printf("#~# Total tris# %d \n",  node->triangles.size());

    root=node;
    return node;
}

void CUVTree::split(sectorNode* inSector)
{

    if(inSector->triangles.size()>50)
    {
        core::vector3df unit=(inSector->dimenBox.MaxEdge-inSector->dimenBox.MinEdge)*0.5f;
        inSector->nodes = new sectorNode[4];

        for(int y=0; y<2; y++)
        {
            for(int x=0; x<2; x++)
            {
                core::vector3df minedge =core::vector3df(inSector->dimenBox.MinEdge.X+(x*unit.X),inSector->dimenBox.MinEdge.Y+(y*unit.Y),-1.0f);
                core::vector3df maxedge =core::vector3df(inSector->dimenBox.MinEdge.X+((x+1)*unit.X),inSector->dimenBox.MinEdge.Y+((y+1)*unit.Y),1.0f);

                inSector->nodes[(x*2)+y].parent=inSector;
                inSector->nodes[(x*2)+y].dimenBox=core::aabbox3df(minedge.X,minedge.Y,minedge.Z ,maxedge.X,maxedge.Y,maxedge.Z);

                for(int t=0; t<inSector->triangles.size(); t++)
                {
                    if(inSector->nodes[(x*2)+y].dimenBox.isPointInside(inSector->triangles[t].pointA))
                    {
                        inSector->nodes[(x*2)+y].triangles.push_back(inSector->triangles[t]);
                        inSector->nodes[(x*2)+y].triangles3D.push_back(inSector->triangles3D[t]);
                    }
                }

            }
        }


        split(&inSector->nodes[0]);
        split(&inSector->nodes[1]);
        split(&inSector->nodes[2]);
        split(&inSector->nodes[3]);

        nlist.push_back(inSector);
        inSector->triangles.clear();

    }
    else
    {
        for(int t=0; t<inSector->triangles.size(); t++)
        {
            inSector->testBox.addInternalPoint(inSector->triangles[t].pointA);
            inSector->testBox.addInternalPoint(inSector->triangles[t].pointB);
            inSector->testBox.addInternalPoint(inSector->triangles[t].pointC);
        }
        inSector->isLeaf=true;
        leaflist.push_back(inSector);
    }

}

void CUVTree::relaxBoundries()
{
    for(int i=0; i<leaflist.size(); i++)
    {
        sectorNode* parent=leaflist[i]->parent;

        while(parent)
        {
            parent->testBox.addInternalPoint(leaflist[i]->testBox.MinEdge);
            parent->testBox.addInternalPoint(leaflist[i]->testBox.MaxEdge);
            parent=parent->parent;
        }
    }
    printf("#~#leaf count# %d \n",  leaflist.size());
}

//TODO: this needs to traverse tree rather than chck all leaves.
topoData CUVTree::getTriangle(const core::vector3df point)
{
    for(int i=0; i<leaflist.size(); i++)
    {
        if(leaflist[i]->testBox.isPointInside(point))
        {
            for(int t=0; t<leaflist[i]->triangles.size(); t++)
            {
                if(leaflist[i]->triangles[t].isPointInsideFast(point))
                {
                    topoData tdata;
                    tdata.tri2D=leaflist[i]->triangles[t];
                    tdata.tri3D=leaflist[i]->triangles3D[t];
                    return tdata;
                }
            }

        }
    }
}
