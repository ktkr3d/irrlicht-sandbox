#include "irrmmd.h"
#include "CPMDMeshFileLoader.h"
#include "CMQOMeshFileLoader.h"

namespace irr {

void irrMMDsetup(IrrlichtDevice *device)
{
	{
		// add pmd loader
		scene::CPMDMeshFileLoader *loader=
			new scene::CPMDMeshFileLoader(device->getSceneManager());
		device->getSceneManager()->addExternalMeshLoader(loader);
		loader->drop();
	}

	{
		// add mqo loader
		scene::CMQOMeshFileLoader *loader=
			new scene::CMQOMeshFileLoader(device->getSceneManager());
		device->getSceneManager()->addExternalMeshLoader(loader);
		loader->drop();
	}
}

namespace scene {

ICameraSceneNode *irrMMDaddRokuroCamera(IrrlichtDevice *device, f32 distance)
{
	ICameraSceneNode* camera = 
		device->getSceneManager()->addCameraSceneNode();
	CSceneNodeAnimatorCameraRokuro* animator=
		new CSceneNodeAnimatorCameraRokuro(
				device->getCursorControl());
	camera->addAnimator(animator);
	animator->setDistance(distance);
	animator->drop();
	return camera;
}

} // namespace scene
} // namespace irr

