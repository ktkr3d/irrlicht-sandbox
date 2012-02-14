#ifndef TYPES_H
#define TYPES_H

#include <irrlicht.h>

using namespace irr;
//extract float inetsnity from compressed hdr, for cutting off dark radisoity lights
inline f32 HDRIntensity(const video::SColor col)
{
    video::SColorf colf=video::SColorf(col.getRed()/255.0f,col.getGreen()/255.0f,col.getBlue()/255.0f,col.getAlpha()/255.0f);
    colf.r*=(65025.0f / 255.0f)/ colf.a;
    colf.g*=(65025.0f / 255.0f)/ colf.a;
    colf.b*=(65025.0f / 255.0f)/ colf.a;

    return colf.r+colf.g+colf.b;
}

struct topoData
{
    core::triangle3df tri2D;
    core::triangle3df tri3D;

};


struct texel
{
    core::vector3df pos;
    core::vector2df UVpos;
    core::vector3df normal;

    int done;

};
struct LightMapNode
{
    video::ITexture* lMap;

    video::ITexture* dirMap;
    video::ITexture* indirMap[2];
    video::ITexture* diffMap;

    u16 indirToggle;

    //array of indirect texture texel model space positions
    core::array<texel> posMap;

    //structural data
    core::array<topoData> topoMap;
    core::array<scene::IMeshBuffer*> buffers;

    scene::ISceneNode* node;

    int progress;
    float area;

};
#endif
