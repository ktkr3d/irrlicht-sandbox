#include <irrlicht.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

int main(int argc, char** argv)
{
    SIrrlichtCreationParameters params;
    params.DriverType = video::EDT_OPENGL;
    params.WindowSize = core::dimension2d<u32>(800, 600);
    params.Bits = 32;
    params.Fullscreen = false;
    params.Stencilbuffer = false;
    params.Vsync = false;
    params.AntiAlias = false;

    for(int i = 0; i < argc; i++) {
        if(strcmp(argv[i], "-wid") == 0)
            params.WindowId = (void*)strtol(argv[i+1], NULL, 16);
        if(strcmp(argv[i], "-fullscreen") == 0)
            params.Fullscreen = true;
        if(strcmp(argv[i], "-width") == 0)
            params.WindowSize.Width = strtol(argv[i+1], NULL, 10);
        if(strcmp(argv[i], "-height") == 0)
            params.WindowSize.Height = strtol(argv[i+1], NULL, 10);
    }
    IrrlichtDevice *device = createDeviceEx(params);

	if (!device)
		return 1;

	device->setWindowCaption(L"Hello World! - Irrlicht Engine Demo");

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager* smgr = device->getSceneManager();
	IGUIEnvironment* guienv = device->getGUIEnvironment();

	guienv->addStaticText(L"Hello World! This is the Irrlicht Software renderer!",
		rect<s32>(10,10,260,22), true);

	IAnimatedMesh* mesh = smgr->getMesh("../../media/sydney.md2");
	if (!mesh)
	{
		device->drop();
		return 1;
	}
	IAnimatedMeshSceneNode* node = smgr->addAnimatedMeshSceneNode( mesh );

	if (node)
	{
		node->setMaterialFlag(EMF_LIGHTING, false);
		node->setMD2Animation(scene::EMAT_STAND);
		node->setMaterialTexture( 0, driver->getTexture("../../media/sydney.bmp") );
	}

	smgr->addCameraSceneNode(0, vector3df(0,30,-40), vector3df(0,5,0));

	while(device->run())
	{
		driver->beginScene(true, true, SColor(255,100,101,140));

		smgr->drawAll();
		guienv->drawAll();

		driver->endScene();
	}

	device->drop();

	return 0;
}
