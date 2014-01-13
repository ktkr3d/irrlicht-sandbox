#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <boost/foreach.hpp>

#include <SMesh.h>
#include <SAnimatedMesh.h>
#include <IVideoDriver.h>

#include "mqoloader.h"
#include "utility.h"
#include "CMQOMeshFileLoader.h"

namespace irr {

namespace scene {

using namespace polymesh;

typedef std::map<io::path, video::ITexture*> TEXTURE_MAP;

static void push_vertex(SMeshBuffer *meshBuffer,
		const mqo::Vector3 &v, 
		const mqo::Vector2 &uv, 
		const mqo::RGBA &rgba)
{
	assert(meshBuffer->Vertices.size()<65535);
	assert(meshBuffer->Indices.size()<65535);

	video::S3DVertex vertex;
	// convert to right handed y-up to left handed y-up.
	vertex.Pos.set(v.x, v.y, -v.z);
	vertex.TCoords.set(uv.x, uv.y);
	vertex.Color.set(rgba.a, rgba.r, rgba.g, rgba.b);
	meshBuffer->Vertices.push_back(vertex);
	meshBuffer->Indices.push_back(meshBuffer->Indices.size());
}

static SMeshBuffer* createMeshBuffer(
		mqo::Loader &loader, unsigned int material_index,
		TEXTURE_MAP &texture_map, video::IVideoDriver *driver)
{
	// not found
	SMeshBuffer *meshBuffer=new SMeshBuffer;

	// setup material
	assert(material_index<loader.materials.size());
	mqo::Material &m=loader.materials[material_index];
#ifdef _WIN32
	// DirectX RGBA order ?
	m.vcol=0;
#endif
	video::SMaterial &material=meshBuffer->Material;
	material.DiffuseColor.set(
			m.color.a, m.color.r, m.color.g, m.color.b
			);
	material.AmbientColor.set(
			255, 255, 255, 255
			);
	material.SpecularColor.set(
			255, 255, 255, 255
			);
	material.GouraudShading=false;
	material.Lighting=false;
	if(m.texture!=""){
		// setup texture
		io::path texture_path(
				util::cp932_to_fs(m.texture).c_str());
		texture_path.replace(L'\\', L'/');

		TEXTURE_MAP::iterator found=texture_map.find(texture_path);
		video::ITexture *texture=0;
		if(found==texture_map.end()){
			texture=
				driver->getTexture(texture_path);
			if(texture){
				texture_map.insert(std::make_pair(texture_path, texture));
			}
			else{
				std::wcout << "fail to load: " 
					<< texture_path.c_str() << std::endl;
			}
		}
		else{
			texture=found->second;
		}
		material.setTexture(0, texture);
		material.MaterialType=
			video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
	}
	return meshBuffer;
}

static IAnimatedMesh* buildMesh(
		mqo::Loader &loader, video::IVideoDriver *driver)
{
	int vertexCount=0;
	int triangleCount=0;
	int qudrangleCount=0;
	int originalVertexCount=0;

	// convert to irrlicht mesh.
	// mqo is shared vertex that has different uv (and normal).
	// therefore, the indexed array is expanded here.
	// 
	// index array is [0, 1, 2, 3, 4, 5...]
	SMesh *mesh=new SMesh;
	TEXTURE_MAP texture_map;
	BOOST_FOREACH(mqo::Object &o, loader.objects){
		////////////////////////////////////////////////////////////
		// each mqo object
		////////////////////////////////////////////////////////////
		originalVertexCount+=o.vertices.size();

		std::map<int, SMeshBuffer*> mesh_map;
		BOOST_FOREACH(mqo::Face &f, o.faces){
			////////////////////////////////////////////////////////////
			// each mqo face
			////////////////////////////////////////////////////////////
			// split by material
			int material_index=f.material_index;
			std::map<int, SMeshBuffer*>::iterator found=
				mesh_map.find(material_index);
			SMeshBuffer *meshBuffer=0;
			if(found==mesh_map.end()){
				// not found. new meshBuffer.
				meshBuffer=createMeshBuffer(
						loader, material_index, texture_map, driver);
				mesh_map.insert(std::make_pair(material_index, meshBuffer));
				mesh->MeshBuffers.push_back(meshBuffer);
			}
			else{
				// use found meshBuffer.
				meshBuffer=found->second;
			}
			// material
			mqo::Material &m=loader.materials[material_index];
			// append face
			switch(f.index_count)
			{
			case 3:
				// triangle
				push_vertex(meshBuffer, o.vertices[f.indices[0]], f.uv[0],
						m.vcol ? f.color[0] : m.color);
				push_vertex(meshBuffer, o.vertices[f.indices[1]], f.uv[1],
						m.vcol ? f.color[1] : m.color);
				push_vertex(meshBuffer, o.vertices[f.indices[2]], f.uv[2],
						m.vcol ? f.color[2] : m.color);

				vertexCount+=3;
				triangleCount+=1;
				break;
			case 4:
				// qudrangle
				// triangle 0
				push_vertex(meshBuffer, o.vertices[f.indices[0]], f.uv[0],
						m.vcol ? f.color[0] : m.color);
				push_vertex(meshBuffer, o.vertices[f.indices[1]], f.uv[1],
						m.vcol ? f.color[1] : m.color);
				push_vertex(meshBuffer, o.vertices[f.indices[2]], f.uv[2],
						m.vcol ? f.color[2] : m.color);
				// triangle 1
				push_vertex(meshBuffer, o.vertices[f.indices[2]], f.uv[2],
						m.vcol ? f.color[2] : m.color);
				push_vertex(meshBuffer, o.vertices[f.indices[3]], f.uv[3],
						m.vcol ? f.color[3] : m.color);
				push_vertex(meshBuffer, o.vertices[f.indices[0]], f.uv[0],
						m.vcol ? f.color[0] : m.color);

				vertexCount+=6;
				triangleCount+=2;
				qudrangleCount+=1;
				break;
			}
		}
	}

	// finalize
	mesh->recalculateBoundingBox();
	SAnimatedMesh *animMesh = new SAnimatedMesh();
	animMesh->Type = EAMT_UNKNOWN;
	animMesh->recalculateBoundingBox();
	animMesh->addMesh(mesh);
	mesh->drop();

	// summary
	std::cout 
		<< originalVertexCount << " vertices"
		<< " is expand to " << vertexCount << " vertices" << std::endl
		<< triangleCount << " triangles ("
		<< qudrangleCount << "quadrangles)" << std::endl
		;

	return animMesh;
}

// CMQOMeshFileLoader
////////////////////////////////////////////////////////////
CMQOMeshFileLoader::CMQOMeshFileLoader(ISceneManager* smgr)
: SceneManager(smgr)
{
}

CMQOMeshFileLoader::~CMQOMeshFileLoader()
{
}

bool 
CMQOMeshFileLoader::isALoadableFileExtension(
		const io::path& filename) const
{
	return core::hasFileExtension ( filename, "mqo" );
}

IAnimatedMesh* 
CMQOMeshFileLoader::createMesh(io::IReadFile* file)
{
	const long filesize = file->getSize();
	if (filesize==0){
		std::cout << "empty file" << std::endl;
		return 0;
	}

	// read all file
	std::vector<char> buf(filesize);
	file->read(&buf[0], filesize);

	// parse mqo
	mqo::Loader loader;
	if(!loader.parse(&buf[0], filesize)){
		return 0;
	}

	// go to model directory
	io::path path=file->getFileName();
	io::path directory(util::dirname(path.c_str()).c_str());
	io::path oldDirectory(util::pwd().c_str());
	util::cd(directory.c_str());

	// build mesh
	IAnimatedMesh *animMesh=buildMesh(
			loader, SceneManager->getVideoDriver());

	// restore current directory
	util::cd(oldDirectory.c_str());

	return animMesh;
}

} // namespace scene
} // namespace irr
