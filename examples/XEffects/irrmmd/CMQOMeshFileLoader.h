/**
 * this file is based on COBJMeshFileLoader.h in irrlicht-1.6.
 * The license is the same as irrlicht.
 */
#ifndef __C_MQO_MESH_FILE_LOADER_H_INCLUDED__
#define __C_MQO_MESH_FILE_LOADER_H_INCLUDED__

#include <IMeshLoader.h>
#include <IFileSystem.h>
#include <ISceneManager.h>
#include <irrString.h>
#include <SMeshBuffer.h>
#include <irrMap.h>
#include <path.h>

namespace irr
{
	namespace scene
	{

		//! Meshloader capable of loading mqo meshes.
		class CMQOMeshFileLoader : public IMeshLoader
		{
			ISceneManager* SceneManager;

		public:

			//! Constructor
			CMQOMeshFileLoader(ISceneManager* smgr);

			//! destructor
			virtual ~CMQOMeshFileLoader();

			//! returns true if the file maybe is able to be loaded by this class
			//! based on the file extension (e.g. ".mqo")
			virtual bool isALoadableFileExtension(const io::path& filename) const;

			//! creates/loads an animated mesh from the file.
			//! \return Pointer to the created mesh. Returns 0 if loading failed.
			//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
			//! See IReferenceCounted::drop() for more information.
			virtual IAnimatedMesh* createMesh(io::IReadFile* file);
		};

	} // end namespace scene
} // end namespace irr

#endif
