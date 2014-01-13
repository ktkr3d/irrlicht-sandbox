#ifndef IRR_MMD_H_INCLUDED
#define IRR_MMD_H_INCLUDED

#include <irrlicht.h>
#include "CCustomSkinnedMesh.h"
#include "CVMDCustomSkinMotion.h"
#include "CSceneNodeAnimatorCameraRokuro.h"

namespace irr {

void irrMMDsetup(IrrlichtDevice *device);

namespace scene {

scene::ICameraSceneNode *irrMMDaddRokuroCamera(
		IrrlichtDevice *device, f32 distance);

} // namespace scene
} // namespace irr

#endif // IRR_MMD_H_INCLUDED
