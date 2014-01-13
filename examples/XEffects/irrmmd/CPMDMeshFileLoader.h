#ifndef C_PMD_MESH_FILE_LOADER_H_INCLUDED
#define C_PMD_MESH_FILE_LOADER_H_INCLUDED

#include <irrlicht.h>

namespace irr
{
namespace scene
{

//! Meshloader capable of loading mqo meshes.
	class CPMDMeshFileLoader : public IMeshLoader
	{
		ISceneManager* SceneManager;

	public:
		//! Constructor
		CPMDMeshFileLoader(ISceneManager *smgr);

		//! destructor
		virtual ~CPMDMeshFileLoader();

		//! returns true if the file maybe is able to be loaded by this class
		//! based on the file extension (e.g. ".mqo")
		virtual bool isALoadableFileExtension(const io::path& filename) const;

		//! creates/loads an animated mesh from the file.
		//! \return Pointer to the created mesh. Returns 0 if loading failed.
		//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
		//! See IReferenceCounted::drop() for more information.
		virtual IAnimatedMesh* createMesh(io::IReadFile* file);

	};

}
}

#endif // C_PMD_MESH_FILE_LOADER_H_INCLUDED
