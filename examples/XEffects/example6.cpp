#include <irrlicht.h>
#include <irrlicht.h>
#include <iostream>
#include <irrUString.h>
#include "CGUITTFont.h"
#include "cAudio.h"
#include "irrmmd.h"
#include "XEffects.h"

#pragma comment(lib, "irrlicht.lib")

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 576

// Namespaces.
using namespace irr;
using namespace scene;
using namespace video;
using namespace core;
using namespace gui;

const stringc PMDs[][2] =
{
    {"./media/model/azusa/", "azusa_p2.pmd"},
    {"./media/model/ikamusume/", "ikamusume.pmd"},
    {"./media/model/latmiku/", "Lat_Normal.pmd"}, // 2
//    {"./media/model/latmiku/", "latmiku.pmd"}, // 2
    {"./media/model/miku/", "miku.pmd"},
    {"./media/model/suiginto/", "suiginto.pmd"},
    {"./media/model/yui/", "yui.pmd"},
    {"./media/model/m_GUMI/", "m_GUMI.pmd"},
    {"./media/model/mikuXS/", "mikuXS.pmd"}, // 7
    {"./media/model/colonmiku/", "colonmiku.pmd"},
    {"./media/model/miku7/", "miku7.pmd"},
    {"./media/model/Sakine_v11/", "luka.pmd"},
    {"./media/model/LAYZNER/", "LAYZNER.pmd"},
    {"./media/model/divamiku/", "divamiku.pmd"},
    {"./media/model/eila/", "eila_121004.pmd"}, // 13
};
const int idxPMD = 7;
const int sizeofPMD = 14;

const stringc VMDs[][3] =
{
    {"./media/motion/", "01_01.vmd", "./media/sound/circulation_fuli.ogg"},
    {"./media/motion/", "circulation.vmd", "./media/sound/circulation_fuli.ogg"}, // 1
    {"./media/motion/", "fighting-28.vmd," "./media/sound/circulation_fuli.ogg"},
    {"./media/motion/", "galaxias!.vmd", "./media/sound/galaxias!.ogg"}, // 3
    {"./media/motion/", "nyancat.vmd", "./media/sound/nyancat.ogg"},
    {"./media/motion/", "walk10.vmd", "./media/sound/circulation_fuli.ogg"},
    {"./media/motion/", "nanairo.vmd", "./media/sound/nanairo.ogg"},
    {"./media/motion/", "galaxias_lat.vmd", "./media/sound/galaxias!.ogg"}, // 7
    {"./media/motion/", "galaxias_miku_v2.vmd", "./media/sound/galaxias!.ogg"},
    {"./media/motion/shakeit_vmd/", "shakeit_miku.vmd", "./media/sound/shake_it.ogg"}, // 9
    {"./media/motion/", "kosmos_cosmos.vmd", "./media/sound/bloody_cosmos.ogg"}, // 10
    {"./media/motion/", "miku2_shiteageru.vmd", "./media/sound/miku2_shiteageru.ogg"}, // 11
};
const int idxVMD = 8;
const int sizeofVMD = 12;

const f32 animationSpeed = 30;

namespace irr
{
namespace bullet
{
class CWorld
{
public:
    void update(u32);
};
}
}

// Event receiver
struct SAppContext
{
    IrrlichtDevice *device;
    s32 counter;
    IAnimatedMeshSceneNode* node;
    cAudio::IAudioManager* audioManager;
    cAudio::IAudioSource* bgm;
    bool pause;
    bool bUseXEffects;
    EffectHandler* effect;
    bool bDrawGUI;
    IGUIListBox* listModel;
    IGUIListBox* listMotion;
    IGUICheckBox* checkShadow;
};

// GUI IDs
enum
{
    GUI_ID_QUIT_BUTTON = 101,
    GUI_ID_RESTART_BUTTON,
    GUI_ID_PAUSE_BUTTON,
    GUI_ID_TRANSPARENCY_SCROLL_BAR,
    GUI_ID_MODEL_LISTBOX,
    GUI_ID_MOTION_LISTBOX,
    GUI_ID_SHADOW_CHECKBOX,
    GUI_ID_LOGO_IMAGE,
    GUI_ID_TAB_SETTINGS
};

// Event Receiver
class MyEventReceiver : public IEventReceiver
{
public:
    MyEventReceiver(SAppContext & context) : Context(context) { }

    virtual bool OnEvent(const SEvent& event)
    {
        if (event.EventType == EET_GUI_EVENT)
        {
            s32 id = event.GUIEvent.Caller->getID();
            IGUIEnvironment* env = Context.device->getGUIEnvironment();
            ISceneManager* smgr = Context.device->getSceneManager();

            switch(event.GUIEvent.EventType)
            {

            case EGET_SCROLL_BAR_CHANGED:
                if (id == GUI_ID_TRANSPARENCY_SCROLL_BAR)
                {
                    s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                    for (u32 i=0; i<EGDC_COUNT ; ++i)
                    {
                        SColor col = env->getSkin()->getColor((EGUI_DEFAULT_COLOR)i);
                        col.setAlpha(pos);
                        env->getSkin()->setColor((EGUI_DEFAULT_COLOR)i, col);
                    }
                }
                break;

            case EGET_BUTTON_CLICKED:
                switch(id)
                {
                case GUI_ID_QUIT_BUTTON:
                    Context.device->closeDevice();
                    return true;

                case GUI_ID_RESTART_BUTTON:
                {
                    Context.bgm->stop();
                    Context.node->setAnimationSpeed(0);
                    Context.node->setCurrentFrame(0);
                    Context.bUseXEffects = Context.checkShadow->isChecked();

                    if (Context.node)
                    {
                        if (Context.bUseXEffects)
                            Context.effect->removeShadowFromNode(Context.node);
                        Context.node->remove();
                    }

                    s32 nModel = Context.listModel->getSelected();
                    s32 nMotion = Context.listMotion->getSelected();

                    Context.device->getFileSystem()->addFileArchive(PMDs[nModel][0].c_str());
                    IAnimatedMesh* mesh = smgr->getMesh(PMDs[nModel][0] + PMDs[nModel][1]);
//					irr::bullet::CWorld* world = ((CCustomSkinnedMesh*)mesh)->getBulletWorld();
                    CVMDCustomSkinMotion* motion = new CVMDCustomSkinMotion;
                    if (motion)
                    {
                        motion->load(VMDs[nMotion][0] + VMDs[nMotion][1]);
                        ((CCustomSkinnedMesh*)mesh)->setMotion(motion);
                        motion->drop();
                        motion=NULL;
                    }
                    Context.node = smgr->addAnimatedMeshSceneNode( mesh );
                    Context.node->setScale(vector3df(1.0f, 1.0f, 1.0f));
                    Context.node->setPosition(vector3df(0.0f, 0.0f, 0.0f));
                    Context.node->setAnimationSpeed(30);
                    Context.node->setMaterialFlag(EMF_LIGHTING, false);
                    if (Context.bUseXEffects)
                        Context.effect->addShadowToNode(Context.node);
                    Context.node->setCurrentFrame(0);

                    Context.bgm->stop();
                    Context.bgm = Context.audioManager->create("music", VMDs[nMotion][2].c_str(), true);
                    if (Context.bgm)
                        Context.bgm->play2d(true);
                }
                return true;

                case GUI_ID_PAUSE_BUTTON:
                {
                    if (Context.pause)
                    {
                        Context.node->setAnimationSpeed(30);
                        Context.bgm->play();
                    }
                    else
                    {
                        Context.node->setAnimationSpeed(0);
                        Context.bgm->pause();
                    }
                    Context.pause = Context.pause ? false : true;
                }
                return true;

                default:
                    return false;
                }
                break;

            default:
                break;
            }
        }
        else if (event.EventType == EET_KEY_INPUT_EVENT &&
                 event.KeyInput.Key == KEY_F9 &&
                 event.KeyInput.PressedDown == false)
            Context.bDrawGUI = !Context.bDrawGUI;

        return false;
    }

private:
    SAppContext& Context;
};

int main(int argc, char** argv)
{
    // Application Context
    SAppContext context;
    context.bUseXEffects = true;
    context.counter = 0;
    context.pause = false;
    context.bDrawGUI = true;

    // Irrlicht Device
    SIrrlichtCreationParameters params;
    params.DriverType = video::EDT_OPENGL;
    params.WindowSize = core::dimension2d<u32>(WINDOW_WIDTH, WINDOW_HEIGHT);
    params.Bits = 32;
    params.Fullscreen = false;
    params.Stencilbuffer = true;
    params.Vsync = true;
    params.AntiAlias = true;
    for(int i=0; i<argc; i++)
    {
        if(strcmp(argv[i], "-wid") == 0)
            params.WindowId=(void*)strtol(argv[i+1], NULL, 16);
        if(strcmp(argv[i], "-fullscreen") == 0)
            params.Fullscreen=true;
        if(strcmp(argv[i], "-width") == 0)
            params.WindowSize.Width=strtol(argv[i+1], NULL, 10);
        if(strcmp(argv[i], "-height") == 0)
            params.WindowSize.Height=strtol(argv[i+1], NULL, 10);
    }
    context.device = createDeviceEx(params);
    if(!context.device)
        return 1;
    ISceneManager* smgr = context.device->getSceneManager();
    IVideoDriver* driver = context.device->getVideoDriver();
    IGUIEnvironment* guienv = context.device->getGUIEnvironment();

    // XEffects
    context.effect = new EffectHandler(context.device, driver->getScreenSize(), true, true, true);
    context.effect->setAmbientColor(SColor(255, 100, 100, 100));

    // Font
    gui::CGUITTFont* tt_font = gui::CGUITTFont::createTTFont(guienv, "./media/font/VL-PGothic-Regular.ttf", 12);
    gui::IGUISkin* skin = guienv->getSkin();
    skin->setFont(tt_font);

    // GUI
    position2d<s32> pos;
    rect<s32> box;

    // GUI - Logo
    pos = position2d<s32>(10, 10);
    guienv->addImage(driver->getTexture(L"./media/texture/irrlichtlogo2.png"), pos, true, 0, GUI_ID_LOGO_IMAGE);

	// GUI - Tab control
    box = rect<s32>(0, 0, 200, 350) + position2d<s32>(10, 115);
	gui::IGUITabControl* tabctrl = guienv->addTabControl(box, 0, true, true, GUI_ID_TAB_SETTINGS);
	gui::IGUITab* tabSettings = tabctrl->addTab(L"Settings");
	gui::IGUITab* tabAbout = tabctrl->addTab(L"About");

    // GUI - Buttons
    box = rect<s32>(0, 0, 30, 30) + position2d<s32>(5, 10);
    guienv->addButton(box, tabSettings, GUI_ID_RESTART_BUTTON, L"▶", L"Start Motion");
    box += position2d<s32>(40, 0);
    guienv->addButton(box, tabSettings, GUI_ID_PAUSE_BUTTON, L"■", L"Pause Motion");
    box += position2d<s32>(40, 0);
    guienv->addButton(box, tabSettings, GUI_ID_QUIT_BUTTON, L"✖", L"Exits Program");

    // GUI - Model List
    box = core::rect<s32>(0, 0, 140, 30) + position2d<s32>(5, 45);
    guienv->addStaticText(L"Model:", box, false, false, tabSettings);
    box = core::rect<s32>(0, 0, 183, 98) + position2d<s32>(5, 60);
    context.listModel = guienv->addListBox(box, tabSettings, GUI_ID_MODEL_LISTBOX, true);
    for (int i = 0; i < sizeofPMD; i++)
    {
        context.listModel->addItem(stringw(PMDs[i][1]).c_str());
    }
    context.listModel->setSelected(idxPMD);

    // GUI - Motion List
    box = core::rect<s32>(0, 0, 140, 30) + position2d<s32>(5, 165);
    guienv->addStaticText(L"Motion & BGM:", box, false, false, tabSettings);
    box = core::rect<s32>(0, 0, 183, 98) + position2d<s32>(5, 180);
    context.listMotion = guienv->addListBox(box, tabSettings, GUI_ID_MOTION_LISTBOX, true);
    for (int i = 0; i < sizeofVMD; i++)
    {
        context.listMotion->addItem(stringw(VMDs[i][1]).c_str());
    }
    context.listMotion->setSelected(idxVMD);

    // GUI - Shaow
    box = core::rect<s32>(0, 0, 140, 30) + position2d<s32>(5, 285);
    context.checkShadow = guienv->addCheckBox(context.bUseXEffects, box, tabSettings, GUI_ID_SHADOW_CHECKBOX, L"XEffects Shadow");

    // Model - Node
    irrMMDsetup(context.device);
    context.device->getFileSystem()->addFileArchive(PMDs[idxPMD][0].c_str());
    IAnimatedMesh* mesh = smgr->getMesh(PMDs[idxPMD][0] + PMDs[idxPMD][1]);
    irr::bullet::CWorld* world = ((CCustomSkinnedMesh*)mesh)->getBulletWorld();
    CVMDCustomSkinMotion* motion = new CVMDCustomSkinMotion;
    if (motion)
    {
        motion->load(VMDs[idxVMD][0] + VMDs[idxVMD][1]);
        ((CCustomSkinnedMesh*)mesh)->setMotion(motion);
        motion->drop();
        motion=NULL;
    }
    context.node = smgr->addAnimatedMeshSceneNode( mesh );
    context.node->setScale(vector3df(1.0f, 1.0f, 1.0f));
//    context.node->setPosition(vector3df(5.0f, 0.5f, 3.0f));
    context.node->setPosition(vector3df(0.0f, 0.0f, 0.0f));
    context.node->setAnimationSpeed(30);
    context.node->setMaterialFlag(EMF_LIGHTING, false);
    context.effect->addShadowToNode(context.node, (E_FILTER_TYPE)core::clamp<u32>((u32)2 - '1', 0, 4));

    // Model - Room
    IMeshSceneNode* room = smgr->addMeshSceneNode(smgr->getMesh("./media/map/ShadRoom/ShadRoom.b3d")->getMesh(0));
    room->setScale(vector3df(10.0f, 10.0f, 10.0f));
    room->setPosition(vector3df(0.0f, -10.0f, 0.0f));
    room->setMaterialTexture(0, driver->getTexture("./media/texture/wall.jpg"));
    room->setMaterialFlag(video::EMF_LIGHTING, false);
    context.effect->addShadowToNode(room);

#if 1
    // Model - Stage
    IAnimatedMesh *planemesh = smgr->addHillPlaneMesh("hill", dimension2df(20, 20), dimension2du(1, 1));
    IAnimatedMeshSceneNode* nodeMagicCircle1 = smgr->addAnimatedMeshSceneNode(planemesh);
    nodeMagicCircle1->setPosition(vector3df(0.0f, 0.0f, 0.0f));
    nodeMagicCircle1->setScale(vector3df(2.0f, 2.0f, 2.0f));
    nodeMagicCircle1->getMaterial(0).Lighting = false;
    nodeMagicCircle1->setMaterialTexture(0, driver->getTexture("./media/texture/magic_circle/magic_circle_1.png"));
    nodeMagicCircle1->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL );
    nodeMagicCircle1->setMaterialFlag(video::EMF_LIGHTING, false);
    ISceneNodeAnimator* rotanim1 = smgr->createRotationAnimator(core::vector3df(0,0.1f,0));
    nodeMagicCircle1->addAnimator(rotanim1);

    IAnimatedMeshSceneNode* nodeMagicCircle2 = smgr->addAnimatedMeshSceneNode(planemesh);
    nodeMagicCircle2->setPosition(vector3df(0.0f, 0.0f, 0.0f));
    nodeMagicCircle2->setScale(vector3df(2.0f, 2.0f, 2.0f));
    nodeMagicCircle2->getMaterial(0).Lighting = false;
    nodeMagicCircle2->setMaterialTexture(0, driver->getTexture("./media/texture/magic_circle/magic_circle_2.png"));
    nodeMagicCircle2->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL );
    nodeMagicCircle2->setMaterialFlag(video::EMF_LIGHTING, false);
    ISceneNodeAnimator* rotanim2 = smgr->createRotationAnimator(core::vector3df(0,-0.1f,0));
    nodeMagicCircle2->addAnimator(rotanim2);

    IAnimatedMeshSceneNode* nodeMagicCircle3 = smgr->addAnimatedMeshSceneNode(planemesh);
    nodeMagicCircle3->setPosition(vector3df(0.0f, 0.0f, 0.0f));
    nodeMagicCircle3->setScale(vector3df(2.0f, 2.0f, 2.0f));
    nodeMagicCircle3->getMaterial(0).Lighting = false;
    nodeMagicCircle3->setMaterialTexture(0, driver->getTexture("./media/texture/magic_circle/magic_circle_3.png"));
    nodeMagicCircle3->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL );
    nodeMagicCircle3->setMaterialFlag(video::EMF_LIGHTING, false);
    ISceneNodeAnimator* rotanim3 = smgr->createRotationAnimator(core::vector3df(0,0.1f,0));
    nodeMagicCircle3->addAnimator(rotanim3);

    IAnimatedMeshSceneNode* nodeMagicCircle4 = smgr->addAnimatedMeshSceneNode(planemesh);
    nodeMagicCircle4->setPosition(vector3df(0.0f, 0.0f, 0.0f));
    nodeMagicCircle4->setScale(vector3df(2.0f, 2.0f, 2.0f));
    nodeMagicCircle4->getMaterial(0).Lighting = false;
    nodeMagicCircle4->setMaterialTexture(0, driver->getTexture("./media/texture/magic_circle/magic_circle_4.png"));
    nodeMagicCircle4->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL );
    nodeMagicCircle4->setMaterialFlag(video::EMF_LIGHTING, false);
    ISceneNodeAnimator* rotanim4 = smgr->createRotationAnimator(core::vector3df(0,-0.1f,0));
    nodeMagicCircle4->addAnimator(rotanim4);
    context.effect->addShadowToNode(nodeMagicCircle1);
    context.effect->addShadowToNode(nodeMagicCircle2);
    context.effect->addShadowToNode(nodeMagicCircle3);
    context.effect->addShadowToNode(nodeMagicCircle4);
#endif

#if 1
    // Model - Water
    mesh = smgr->addHillPlaneMesh( "water",
                                   core::dimension2d<f32>(1, 1),
                                   core::dimension2d<u32>(100, 100),
                                   0,
                                   0,
                                   core::dimension2d<f32>(0,0),
                                   core::dimension2d<f32>(10,10));
    scene::ISceneNode* water = smgr->addWaterSurfaceSceneNode(mesh->getMesh(0), 0.1f, 300.0f, 0.1f);
    water->setPosition(vector3df(0.0f, -1.0f, 0.0f));
    water->setMaterialTexture(0, driver->getTexture("./media/texture/stones.jpg"));
    water->setMaterialTexture(1, driver->getTexture("./media/texture/water.jpg"));
    water->setMaterialType(video::EMT_REFLECTION_2_LAYER);
    water->setMaterialFlag(EMF_LIGHTING, false);
    context.effect->addShadowToNode(water);
#endif

    // Lights
//    context.effect->setClearColour(SColor(0x0));
#if 1
    context.effect->addShadowLight(SShadowLight(512, vector3df(-100, 100, -100), vector3df(0, 10, 0),
                                      SColor(255, 255, 255, 255), 20.0f, 1000.0f, 90.0f * DEGTORAD));
#else
    context.effect->addShadowLight(SShadowLight(256, vector3df(-15, 30, -15), vector3df(5, 0, 5)));
    context.effect->addShadowLight(SShadowLight(256, vector3df(15, 30, -15), vector3df(5, 0, 5)));
    context.effect->addShadowLight(
        SShadowLight(
            1024,
            vector3df(-15, 30, -15),
            vector3df(5, 0, 5),
            SColor(0xffffffff),
            10.0f,
            100.0f,
            90.0 * DEGTORAD64,
            false));
#endif

    // Camera
    scene::ICameraSceneNode* camera = scene::irrMMDaddRokuroCamera(context.device, 100.0f);
    camera->setFOV(static_cast<f32>(M_PI*30.0/180.0));
    camera->setNearValue(1.0f);
    camera->setFarValue(1000.0f);
    camera->setTarget(vector3df(0, 10, 0));

    // Timer
    ITimer* timer = context.device->getTimer();
    u32 lastMS = timer->getRealTime();
    u32 difMs = timer->getRealTime() - lastMS;
    lastMS = timer->getRealTime();
    f32 alignSpeed = difMs/16.0;
    s32 oldFps = 0;

    // Set Event Receiver
    MyEventReceiver receiver(context);
    context.device->setEventReceiver(&receiver);

    // Audio
    context.audioManager = cAudio::createAudioManager(true);
    context.audioManager->initialize(context.audioManager->getAvailableDeviceName(0));
    context.bgm = context.audioManager->create("music", VMDs[idxVMD][2].c_str(), true);
    if (context.bgm)
        context.bgm->play2d(true);

    // Main loop
    while(context.device->run())
    {
        if(oldFps != driver->getFPS())
        {
            core::stringw windowCaption = L"VSM Shadow Mapping Demo FPS: ";
            windowCaption += driver->getFPS();
            context.device->setWindowCaption(windowCaption.c_str());
            oldFps = driver->getFPS();
        }
        driver->beginScene(true, true, SColor(0x0));
        if (context.bUseXEffects)
            context.effect->update();
        else
            smgr->drawAll();
        if (context.bDrawGUI)
            guienv->drawAll();
        driver->endScene();
    }

    tt_font->drop();
    context.device->drop();

    return 0;
}
