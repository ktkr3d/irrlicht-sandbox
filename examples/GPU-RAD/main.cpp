#include <irrlicht.h>
#include "driverChoice.h"
#include "XEffects.h"
#include <stdio.h>
#include <iostream>
#include "types.h"
#include "CUVTree.h"
#include "mtrand.h"
using namespace std;

using namespace irr;

IrrlichtDevice *device;
EffectHandler* xeffect;

u32 shadowDimen = 1024;
f32 samplingFactor=3.0f;
u32 occlusionAccuracy = 256;
u32 diffuseSmooth=8;
f32 occlusionAngle=100.0f;

//defualts
int bounces=3;
int accuracy=85;

bool processed =false;
E_FILTER_TYPE filterType = (E_FILTER_TYPE)0;
class ShaderCallBack : public video::IShaderConstantSetCallBack
{
public:

    virtual void OnSetConstants(video::IMaterialRendererServices* services,
                                s32 userData)
    {
        video::IVideoDriver* driver = services->getVideoDriver();

        core::matrix4 invWorld = driver->getTransform(video::ETS_WORLD);

        irr::u32 TexVar = 0;
        services->setPixelShaderConstant("tex", (irr::f32*)(&TexVar), 1);
        irr::u32 TexVarl = 1;
        services->setPixelShaderConstant("tex2", (irr::f32*)(&TexVarl), 1);

    }
};

class CRadiosityGenerator
{
private:

    //random 0.0f->1.0f float gen
    MTRand_open rgen;

    scene::IMeshBuffer* mb;

    scene::IMesh* mesh;

    LightMapNode* currentlNodes;

    core::array<LightMapNode*> lNodes;
    //core::array<video::ITexture*> hemiCubes;

    u32 totalHemiCubes;

    u32 totalCurrentLightMaps;
    u32 totalLightMaps;
    u32 totalDiffuseMaps;

    //post processor node
    scene::IMeshSceneNode* ppNode;
    scene::IMeshSceneNode* texelNode;

    //
    s32 postDisplay;
    s32 diffGrab;
    s32 addl;
    s32 finalise;

    scene::ISceneManager* smgr;
    //scene::IMetaTriangleSelector * megaSelector;
    video::IGPUProgrammingServices* gpu;
    ShaderCallBack* shCallback;
    EffectHandler* effect;

    scene::ICameraSceneNode* hemiCam;

    /////////////////////////////////////////////////////////////////////////
    void groupNodes(scene::ISceneNode* inNode)
    {
        printf("#=# Organising data");
        //Create LightMap nodes
        core::map<video::ITexture*,bool> hashtex;
        for (int i =0; i<mesh->getMeshBufferCount(); i++)
        {
            mb=mesh->getMeshBuffer(i);

            if (mb->getMaterial().getTexture(1))
            {
                hashtex[mb->getMaterial().getTexture(1)]=true;
            }
        }

        totalCurrentLightMaps = hashtex.size();
        currentlNodes = new LightMapNode[totalCurrentLightMaps];

        hashtex.clear();
        int count=0;
        for (int i =0; i<mesh->getMeshBufferCount(); i++)
        {
            mb=mesh->getMeshBuffer(i);

            if (mb->getMaterial().getTexture(1))
            {
                if (hashtex.insert(mb->getMaterial().getTexture(1),true))
                {
                    currentlNodes[count].node=inNode;
                    currentlNodes[count].lMap=mb->getMaterial().getTexture(1);

                    currentlNodes[count].diffMap = smgr->getVideoDriver()->addRenderTargetTexture(currentlNodes[count].lMap->getSize()/diffuseSmooth,"test", video::ECF_A8R8G8B8);
                    currentlNodes[count].dirMap = smgr->getVideoDriver()->addRenderTargetTexture(currentlNodes[count].lMap->getSize(),"test", video::ECF_A8R8G8B8);
                    currentlNodes[count].indirMap[0] = smgr->getVideoDriver()->addRenderTargetTexture(currentlNodes[count].lMap->getSize(),"test", video::ECF_A8R8G8B8);
                    currentlNodes[count].indirMap[1] = smgr->getVideoDriver()->addRenderTargetTexture(currentlNodes[count].lMap->getSize(),"test", video::ECF_A8R8G8B8);

                    currentlNodes[count].indirToggle=0;
                    currentlNodes[count].progress=0;
                    for (int k =0; k<mesh->getMeshBufferCount(); k++)
                    {
                        mb=mesh->getMeshBuffer(k);
                        if ( currentlNodes[count].lMap==mb->getMaterial().getTexture(1))
                        {
                            mb->setHardwareMappingHint(scene::EHM_STATIC,scene::EBT_VERTEX_AND_INDEX);
                            currentlNodes[count].buffers.push_back(mb);
                            printf("...");
                        }
                    }

                    count+=1;

                }
            }
        }

        printf("\n ~=~ Total Light-Maps %d \n", count);
        //printf("#=# Building collision trees");
        for (int i=0; i<totalCurrentLightMaps; i++)
        {
            for (int k =0; k<currentlNodes[i].buffers.size(); k++)
            {
                for (int z =0; z<currentlNodes[i].node->getMaterialCount(); z++)
                {
                    if (currentlNodes[i].node->getMaterial(z)==currentlNodes[i].buffers[k]->getMaterial())
                    {
                        currentlNodes[i].node->getMaterial(z).setTexture(1,currentlNodes[i].dirMap);
                        currentlNodes[i].node->getMaterial(z).BackfaceCulling=false;
                        currentlNodes[i].buffers[k]->getMaterial().setTexture(1,currentlNodes[i].dirMap);
                        currentlNodes[i].buffers[k]->getMaterial().BackfaceCulling=false;
                    }

                }
            }

        }

    }

    /////////////////////////////////////////////////////////////////////////
    void extractDiffuseData()
    {
        video::IVideoDriver* driver = smgr->getVideoDriver();
        for (int i =0; i<totalCurrentLightMaps; i++)
        {
            video::SMaterial diffMaterial;

            driver->setRenderTarget(currentlNodes[i].diffMap, true, true, video::SColor(255,0,0,0));

            for (int z=0; z<currentlNodes[i].buffers.size(); z++)
            {
                diffMaterial=currentlNodes[i].buffers[z]->getMaterial();
                diffMaterial.MaterialType=(video::E_MATERIAL_TYPE)diffGrab;
                diffMaterial.BackfaceCulling=false;
                driver->setMaterial(diffMaterial);

                driver->drawMeshBuffer(currentlNodes[i].buffers[z]);
            }
            driver->setRenderTarget(0,true, true);


        }
        printf("\n");
    }
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
    core::vector3df UVtoWorldPos(const core::vector3df point,const topoData& tdat)
    {
        core::vector3df rayBC=tdat.tri2D.pointC-tdat.tri2D.pointB;
        core::vector3df uvRay=-point+tdat.tri2D.pointA;
        core::vector3df bNormal=core::vector3df(rayBC.normalize().Y,-rayBC.normalize().X,0);

        f32  D = - tdat.tri2D.pointB.dotProduct(bNormal);

        f32 t2 = bNormal.dotProduct(uvRay.normalize());

        f32 t =- (bNormal.dotProduct(point) + D) / t2;
        core::vector3df outPt= point + (uvRay.normalize() * t);

        core::line3df lineBout=core::line3df(tdat.tri2D.pointB,outPt);
        core::line3df lineBC=core::line3df(tdat.tri2D.pointB,tdat.tri2D.pointC);

        core::line3df lineAuv=core::line3df(tdat.tri2D.pointA,point);
        core::line3df lineAout=core::line3df(tdat.tri2D.pointA,outPt);

        f32 factorBout= lineBout.getLength()/lineBC.getLength();
        f32 factorAout= lineAuv.getLength()/lineAout.getLength();

        core::line3df line3DBC=core::line3df(tdat.tri3D.pointB,tdat.tri3D.pointC);
        core::vector3df out3D=tdat.tri3D.pointB+(line3DBC.getVector().normalize()*line3DBC.getLength()*factorBout);

        core::line3df line3DAout=core::line3df(tdat.tri3D.pointA,out3D);
        core::vector3df final3D=tdat.tri3D.pointA+(line3DAout.getVector().normalize()*line3DAout.getLength()*factorAout);

        return final3D;
    }
/////////////////////////////////////////////////////////////////////////
    void uvtopos()
    {
        //generate topo structures
        for (int i =0; i< totalCurrentLightMaps; i++)
        {

            for (int k =0; k< currentlNodes[i].buffers.size(); k++)
            {
                for (int j =0; j< currentlNodes[i].buffers[k]->getIndexCount(); j+=3)
                {
                    topoData lMapTopo;

                    const u16 vtx1 =currentlNodes[i].buffers[k]->getIndices()[j];
                    const u16 vtx2 =currentlNodes[i].buffers[k]->getIndices()[j+1];
                    const u16 vtx3 =currentlNodes[i].buffers[k]->getIndices()[j+2];

                    video::S3DVertex2TCoords* vtxp=(video::S3DVertex2TCoords*)currentlNodes[i].buffers[k]->getVertices();

                    core::vector3df vtx1uv= core::vector3df(vtxp[vtx1].TCoords2.X,vtxp[vtx1].TCoords2.Y,0.0f);
                    core::vector3df vtx2uv=core::vector3df(vtxp[vtx2].TCoords2.X,vtxp[vtx2].TCoords2.Y,0.0f);
                    core::vector3df vtx3uv=core::vector3df(vtxp[vtx3].TCoords2.X,vtxp[vtx3].TCoords2.Y,0.0f);
                    lMapTopo.tri2D.set(vtx1uv,vtx2uv,vtx3uv);
                    lMapTopo.tri3D.set(currentlNodes[i].buffers[k]->getPosition(vtx1),currentlNodes[i].buffers[k]->getPosition(vtx2),currentlNodes[i].buffers[k]->getPosition(vtx3));
                    currentlNodes[i].topoMap.push_back(lMapTopo);
                }
            }
            printf("...");

        }
        printf("\n");

        printf("#=# Building UVmap Texel grid\n");

        int blockCount =0;
        //calculate texel poitions

        for (int i =0; i< totalCurrentLightMaps; i++)
        {
            printf("#=# Creating UVmap Tree\n");
            CUVTree tree(currentlNodes[i].topoMap);
            //Calculate area
            currentlNodes[i].area=0.0f;
            for (int t=0; t<currentlNodes[i].topoMap.size(); t++)
            {
                currentlNodes[i].area+=currentlNodes[i].topoMap[t].tri3D.getArea();
            }
            core::dimension2du mapSize=core::dimension2du(sqrt(currentlNodes[i].area/samplingFactor),sqrt(currentlNodes[i].area/samplingFactor));

            for (int x =0; x< mapSize.Width; x++)
            {
                for (int y =0; y< mapSize.Height; y++)
                {
                    //find which triangle texel belongs to

                    core::vector3df uvPoint=core::vector3df(f32(x)/mapSize.Width,f32(y)/mapSize.Height,0);
                    topoData tridata=tree.getTriangle(uvPoint);
                    texel tdata;
                    tdata.done=0;
                    tdata.pos=UVtoWorldPos(uvPoint, tridata);
                    tdata.UVpos=core::vector2df(uvPoint.X,uvPoint.Y);
                    tdata.normal= tridata.tri3D.getNormal();
                    currentlNodes[i].posMap.push_back(tdata);
                    //smgr->addCubeSceneNode(1)->setPosition(tdata.pos);

                    /*for (int t=0; t<currentlNodes[i].topoMap.size(); t++)
                    {
                        //printf("texture\n");
                        if (currentlNodes[i].topoMap[t].tri2D.isPointInsideFast(uvPoint))
                        {
                            blockCount++;
                            texel tdata;
                            tdata.done=0;
                            tdata.pos=UVtoWorldPos(uvPoint,currentlNodes[i].topoMap[t]);
                            tdata.UVpos=core::vector2df(uvPoint.X,uvPoint.Y);
                            tdata.triTopo=&currentlNodes[i].topoMap[t];
                            tdata.normal=tdata.triTopo->tri3D.getNormal();
                            currentlNodes[i].posMap.push_back(tdata);
                            //smgr->addCubeSceneNode(1)->setPosition(tdata.pos);
                            //if (blockCount>10000) return 0;
                        }
                    }*/
                }
            }
            printf("...");
        }
        printf("\n");
    }

public:
    ~CRadiosityGenerator()
    {
        clearSectorMemory();
        shCallback->drop();
        ppNode->remove();
    }
    CRadiosityGenerator(scene::ISceneManager* mgr, EffectHandler* _effect)
    {
        effect=_effect;
        totalLightMaps=0;
        smgr=mgr;

        //setup random generator
        // 32-bit int generator and initial seed: super secret code ;)
        unsigned long init[8] = {0x6f,0x6d,0x61,0x72,0x65,0x6d,0x61,0x64}, length = 8;
        MTRand_int32 irand(init, length);

        //megaSelector = mgr->createMetaTriangleSelector();
        ppNode = smgr->addCubeSceneNode();
        ppNode->setVisible(false);
        ppNode->setAutomaticCulling(irr::scene::EAC_OFF);

        gpu = mgr->getVideoDriver()->getGPUProgrammingServices();

        shCallback = new ShaderCallBack();


        diffGrab = gpu->addHighLevelShaderMaterialFromFiles(
                       "media/shader/ppVert.glsl", "vertexMain", video::EVST_VS_1_1,
                       "media/shader/ppFrag.glsl", "pixelMain", video::EPST_PS_1_1,
                       shCallback, video::EMT_SOLID);
        addl = gpu->addHighLevelShaderMaterialFromFiles(
                   "media/shader/ppAddV.glsl", "vertexMain", video::EVST_VS_1_1,
                   "media/shader/ppAddF.glsl", "pixelMain", video::EPST_PS_1_1,
                   shCallback, video::EMT_SOLID);
        finalise = gpu->addHighLevelShaderMaterialFromFiles(
                       "media/shader/ppFinalV.glsl", "vertexMain", video::EVST_VS_1_1,
                       "media/shader/ppFinalF.glsl", "pixelMain", video::EPST_PS_1_1,
                       shCallback, video::EMT_SOLID);
    }

    void addRadiosityNode(scene::IMeshSceneNode* inNode)
    {
        mesh = inNode->getMesh();
        groupNodes(inNode);
        printf("#=# Calculating Texel positions");
        uvtopos();
        printf("#=# Extracting diffuse data");
        extractDiffuseData();

        for (int i=0; i<totalCurrentLightMaps; i++)
        {
            lNodes.push_back(&currentlNodes[i]);
        }
        totalLightMaps+=totalCurrentLightMaps;

        effect->addShadowToNode(inNode, filterType);
    }
/////////////////////////////////////////////////////////////////////////
    core::vector3df calculateDirectLighting(const u32 timeIn)
    {
        video::IVideoDriver* driver = smgr->getVideoDriver();

        for (int i =0; i<totalLightMaps; i++)
        {
            //lNodes[i].node->setVisible(true);
            //driver->setRenderTarget(lNodes[i].dirMap, true, true, video::SColor(255,255,255,255));
            effect->update(timeIn,true,lNodes[i]->dirMap,lNodes[i]->node,&lNodes[i]->buffers);
            driver->setRenderTarget(0);
            //lNodes[i].node->setVisible(false);
        }
    }

    /////////////////////////////////////////////////////////////////////////
    int calculateInDirectLighting(int pass, irr::ITimer* timerg)
    {
        video::IVideoDriver* driver = smgr->getVideoDriver();

        printf("#=# Bouncing.....\n");
        rgen.seed(timerg->getTime());
        effect->removeShadowLights();
        LightMapNode * lnode=0;
        for(int i=0; i<totalLightMaps; i++)
        {
            lnode=lNodes[i];
            //make a backup copy of the last bounce map, or if first pass, the direct lightmap
            driver->setRenderTarget(lnode->indirMap[lnode->indirToggle],true, true, video::SColor(0,0,0,0));
            if(pass==0)
                driver->draw2DImage((video::ITexture*)lnode->dirMap,core::position2di(0,0));
            else
            {
                if(lnode->indirToggle==0)
                {
                    driver->draw2DImage((video::ITexture*)lnode->indirMap[1],core::position2di(0,0));

                }
                else
                    driver->draw2DImage((video::ITexture*)lnode->indirMap[0],core::position2di(0,0));
            }

            driver->setRenderTarget(0);

//            u8* textureloc = (u8*)lnode->dirMap->lock(true,0);
            u8* textureloc = (u8*)lnode->dirMap->lock(video::ETLM_READ_WRITE,0);
//            u8* dtextureloc = (u8*)lnode->diffMap->lock(true,0);
            u8* dtextureloc = (u8*)lnode->diffMap->lock(video::ETLM_READ_WRITE,0);
            for (int i=0; i<lnode->posMap.size()*accuracy/100.0f; i++)
            {
                f32 randomn=(f32)rgen() ;
                u32 id=randomn*lnode->posMap.size();

                //if we already shot light from this texel look for a unprocessed one
                if(lnode->posMap[id].done>1)
                    while(1)
                    {
                        f32 randomn=(f32)rgen() ;
                        id=randomn*lnode->posMap.size();
                        if(lnode->posMap[id].done<1)
                        {
                            break;
                        }
                    }
                texel t=lnode->posMap[id];

                //read back lighting and diffuse data to create indirect lights
                int xPos=t.UVpos.X*lnode->dirMap->getSize().Width;
                int yPos=t.UVpos.Y*lnode->dirMap->getSize().Height;
                int index=(yPos*(lnode->dirMap->getSize().Width*4))+(xPos*4);

                int dxPos=t.UVpos.X*lnode->diffMap->getSize().Width;
                int dyPos=t.UVpos.Y*lnode->diffMap->getSize().Height;
                int dindex=(dyPos*(lnode->diffMap->getSize().Width*4))+(dxPos*4);

                //Decay photons by surface properties
                video::SColor diffusecol=video::SColor(0,dtextureloc[dindex+2],dtextureloc[dindex+1],dtextureloc[dindex]);
                video::SColor col=video::SColor(textureloc[index+3],textureloc[index+2],textureloc[index+1],textureloc[index]);
                diffusecol=video::SColor(textureloc[index+3]/0.15f,diffusecol.getRed()*col.getRed()*0.0005f,diffusecol.getGreen()*col.getGreen()*0.0005f,diffusecol.getBlue()*col.getBlue()*0.0005f);

                //skip dark texels
                if(HDRIntensity(diffusecol)>1.0f)
                {
                    effect->addShadowLight(SShadowLight(occlusionAccuracy, t.pos, t.pos+t.normal,diffusecol, 0.01f, 40.0f, occlusionAngle*core::DEGTORAD,false,true));
                    //smgr->addCubeSceneNode(1)->setPosition(t.pos);
                    effect->getShadowLight(effect->getShadowLightCount()-1).setPosition(t.pos);
                    effect->getShadowLight(effect->getShadowLightCount()-1).setTarget(t.pos+t.normal);
                }
            }

            lnode->dirMap->unlock();
            lnode->diffMap->unlock();
        }
        //bounce the light
        this->calculateDirectLighting(timerg->getTime());

        //add new lighting results to last bounce
        for(int i=0; i<totalLightMaps; i++)
        {
            lnode=lNodes[i];
            for(int i=0; i<lnode->posMap.size(); i++)
            {
                lnode->posMap[i].done=0;
            }

            if(lnode->indirToggle==1)
                driver->setRenderTarget(lnode->indirMap[0],true, true, video::SColor(0,0,0,0));
            else
                driver->setRenderTarget(lnode->indirMap[1],true, true, video::SColor(0,0,0,0));
            {
                ppNode->setMaterialTexture( 0,(video::ITexture*)lnode->dirMap);
                ppNode->setMaterialTexture( 1,lnode->indirMap[lnode->indirToggle]);
            }
            ppNode->setMaterialTexture( 1,lnode->indirMap[lnode->indirToggle]);
            ppNode->setMaterialType((video::E_MATERIAL_TYPE)addl);
            ppNode->setVisible(true);
            ppNode->render();
            ppNode->setVisible(false);
            driver->setRenderTarget(0);

            if(lnode->indirToggle==0)
                lnode->indirToggle=1;
            else
                lnode->indirToggle=0;
        }
        //remove indirect lights from this bounce
        effect->removeShadowLights();
    }

    void saveLightMaps()
    {
        video::IVideoDriver* driver = smgr->getVideoDriver();

        for (int i =0; i<totalLightMaps; i++)
        {
            int val;
            if(lNodes[i]->indirToggle==1)
            {
                val=0;
                driver->setRenderTarget(lNodes[i]->indirMap[0],true, true, video::SColor(0,0,0,0));
            }
            else
            {
                val=1;
                driver->setRenderTarget(lNodes[i]->indirMap[1],true, true, video::SColor(0,0,0,0));
            }


            ppNode->setMaterialTexture( 1,lNodes[i]->indirMap[lNodes[i]->indirToggle]);
            ppNode->setMaterialTexture( 0,lNodes[i]->indirMap[lNodes[i]->indirToggle]);

            ppNode->setMaterialType((video::E_MATERIAL_TYPE)finalise);
            ppNode->setVisible(true);
            ppNode->render();
            ppNode->setVisible(false);
            driver->setRenderTarget(0);

            video::IImage* image = driver->createImageFromData (
                                       lNodes[i]->indirMap[val]->getColorFormat(),
                                       lNodes[i]->indirMap[val]->getSize(),
                                       lNodes[i]->indirMap[val]->lock(),
                                       false  //copy mem
                                   );

            lNodes[i]->indirMap[val]->unlock();

            driver->writeImageToFile(image,lNodes[i]->lMap->getName());
            image->drop();

        }

        printf("#=# Lightmaps saved :)\n");
        for (int i=0; i<totalLightMaps; i++)
        {
            for (int k =0; k<lNodes[i]->buffers.size(); k++)
            {
                for (int z =0; z<lNodes[i]->node->getMaterialCount(); z++)
                {
                    if (lNodes[i]->node->getMaterial(z)==lNodes[i]->buffers[k]->getMaterial())
                    {
                        lNodes[i]->node->getMaterial(z).setTexture(1,lNodes[i]->indirMap[lNodes[i]->indirToggle]);
                        //inNode->getMaterial(z).BackfaceCulling=false;
                        //inNode->getMaterial(z).ZWriteEnable=;
                        //lNodes[i]->node->getMaterial(z).MaterialType=video::EMT_LIGHTMAP;

                        //inNode->getMaterial(z).TextureLayer[0].TrFilter=0;
                        //inNode->getMaterial(z).TextureLayer[0].BilinearFilter=0;
                        //inNode->getMaterial(z).TextureLayer[1].BilinearFilter=0;

                    }

                }
            }

        }

    }

    void clearSectorMemory()
    {
        for (int i=0; i<lNodes.size(); i++)
        {
            smgr->getVideoDriver()->removeTexture(lNodes[i]->lMap);
            smgr->getVideoDriver()->removeTexture(lNodes[i]->diffMap);
            smgr->getVideoDriver()->removeTexture(lNodes[i]->dirMap);
            smgr->getVideoDriver()->removeTexture(lNodes[i]->indirMap[0]);
            smgr->getVideoDriver()->removeTexture(lNodes[i]->indirMap[1]);
            delete lNodes[i];
        }
    }
};
CRadiosityGenerator *myNode;
scene::ISceneManager* smgr;



class MyEventReceiver : public IEventReceiver
{
public:
    // This is the one method that we have to implement
    virtual bool OnEvent(const SEvent& event)
    {
        // Remember whether each key is down or up
        if (event.EventType == irr::EET_KEY_INPUT_EVENT)
            KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;

        if (((event.EventType == EET_KEY_INPUT_EVENT &&
                event.KeyInput.Key == KEY_SPACE &&
                event.KeyInput.PressedDown == false) ||
                (event.EventType == EET_MOUSE_INPUT_EVENT &&
                 event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP))
           )
        {

            if(!processed)
            {
                //
                myNode->calculateDirectLighting(device->getTimer()->getTime());
                myNode->calculateInDirectLighting(0,device->getTimer());
                for(int i=0; i<bounces-1; i++)
                    myNode->calculateInDirectLighting(bounces,device->getTimer());
                //myNode->calculateInDirectLighting(myNode->lNodes[0],2,device->getTimer());
                myNode->saveLightMaps();
                processed =true;
                //printf("camera X %f %f %f\n",smgr->getActiveCamera()->getAbsolutePosition().X,smgr->getActiveCamera()->getAbsolutePosition().Y,smgr->getActiveCamera()->getAbsolutePosition().Z);
            }
        }

        return false;
    }

    // This is used to check whether a key is being held down
    virtual bool IsKeyDown(EKEY_CODE keyCode) const
    {
        return KeyIsDown[keyCode];
    }

    MyEventReceiver()
    {
        for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
            KeyIsDown[i] = false;
    }

private:
    // We use this array to store the current state of each key
    bool KeyIsDown[KEY_KEY_CODES_COUNT];
};

int main()
{
    cout << "#Amun-Ra-diosity# GPU powered radiosity processor (c)OmarEmad\n";
    cout << "When the program loads you will see direct lighting only \n";
    cout << "Click your mouse to generate the rad for that snapshot of time \n";
    cout << "Please enter number of light bounces (3 is more than enought for most cases)";
    cin >> bounces;

    cout << "Please enter accuracy (integer 0-100, 85 is good lower if you want a quick test)";
    cin >> accuracy;

    MyEventReceiver receiver;

    device = createDevice(video::EDT_OPENGL,core::dimension2d<u32>(640,480), 32,false, false, false, &receiver);

    if (device == 0)
        return 1; // could not create selected driver.

    // create engine and camera
    device->getLogger()->setLogLevel(ELL_NONE);
    device->setWindowCaption(L"Custom Scene Node - Irrlicht Engine Demo");

    video::IVideoDriver* driver = device->getVideoDriver();
    smgr = device->getSceneManager();
    xeffect = new EffectHandler(device, driver->getScreenSize());

    device->getFileSystem()->addFileArchive("media/model/MainBuilding");
    scene::IMeshSceneNode*node=smgr->addMeshSceneNode(smgr->getMesh("media/model/MainBuilding/MainBuilding.irrmesh"));
    myNode =new CRadiosityGenerator(smgr,xeffect);

    myNode->addRadiosityNode(node);


    xeffect->setAmbientColor(video::SColor(0,0,0,0));
    scene::ILightSceneNode* light = smgr->addLightSceneNode();
    smgr->addCubeSceneNode(1,light)->setMaterialFlag(video::EMF_LIGHTING,false);

    light->setPosition(core::vector3df(1, 20, 05));
    light->addAnimator(smgr->createFlyCircleAnimator(core::vector3df(1, 20, 05),15));
    // A fairly close farValue is ok for this point light.
    f32 farValue =50.0f;
    //node->setPosition(core::vector3df(-1300,-144,-1249));
    smgr->addCameraSceneNodeFPS(0, 100.0f, 0.05f);

    // The FOV is nearly 90 degrees. I set it a little lower to make the seems less visible.
    f32 fov = 89.999999* core::DEGTORAD;
    for (u32 i = 0; i < 1; ++i)
    {
        //NB: alpha is inverse light power, light decays to zero at far value.
        xeffect->addShadowLight(SShadowLight(shadowDimen, core::vector3df(0,0,0), core::vector3df(0,0,0),
                                             video::SColor(200,240,200,128), 1.0f, farValue, fov,false,true));
    }

    u32 oldFps;
    u32 frames=0;

    while (device->run())
    {
        if (oldFps != driver->getFPS())
        {
            core::stringw windowCaption = L"Light Map Demo";
            device->setWindowCaption(windowCaption.c_str());
            oldFps = driver->getFPS();
        }
        frames++;
        if (!processed&&frames>10)
        {

            //light->setPosition(smgr->getActiveCamera()->getPosition()+ core::vector3df(4,3,-1));
            frames=0;
            for (u32 i = 0; i < 1; ++i)
                xeffect->getShadowLight(i).setPosition(light->getAbsolutePosition());
            // Get the shadow lights and set their positions to the positions
            // of the light scene nodes.
//            effect->getShadowLight(0).setTarget(smgr->getActiveCamera()->getPosition() + vector3df(0,0,1));
            //
            ///*
            //xeffect->getShadowLight(0).setTarget(light->getAbsolutePosition() + core::vector3df(0,0,-1));
            xeffect->getShadowLight(1).setTarget(light->getAbsolutePosition()  + core::vector3df(0,0,1));
            xeffect->getShadowLight(2).setTarget(light->getAbsolutePosition()  + core::vector3df(1,0,0));
            xeffect->getShadowLight(3).setTarget(light->getAbsolutePosition() + core::vector3df(-1,0,0));
            xeffect->getShadowLight(4).setTarget(light->getAbsolutePosition() + core::vector3df(0,1,0));
            xeffect->getShadowLight(5).setTarget(light->getAbsolutePosition()  + core::vector3df(0,-1,0));
//*/
            myNode->calculateDirectLighting(device->getTimer()->getTime());
            // myNode->setVisible(false);
        }
        driver->beginScene();
        smgr->drawAll();

        //node->setVisible(true);
//        myNode->drawTexel(0.5,1,0,0.25);
        //drawTexel(0.5,1,1,0.125);

        //drawTexel(0.5,0.9,0,0.25);
        //drawTexel(0.5,0.9,1,0.125);

        //effect->update(device->getTimer()->getTime(),0);
//driver->setRenderTarget(0);
        //myNode->render();
        driver->endScene();

    }
    delete myNode;
    device->drop();

    return 0;
}

