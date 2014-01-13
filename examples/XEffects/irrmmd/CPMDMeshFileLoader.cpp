#include "CPMDMeshFileLoader.h"
#include "CCustomSkinnedMesh.h"
#include "CJoint.h"
#include "CShape.h"
#include "CRigidBody.h"
#include "pmdloader.h"
#include "utility.h"
#include <map>

#include "irrbullet.h"

// VCÌxñð
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning( disable : 4819 )
#endif
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

inline std::ostream& operator<<(std::ostream &os, const btVector3 &rhs)
{
	return os
		<< '[' << rhs.x() << ',' << rhs.y() << ',' << rhs.z() << ']'
		<< std::endl
		;
}
inline std::ostream& operator<<(std::ostream &os, const btMatrix3x3 &rhs)
{
	return os
		<< rhs[0] << rhs[1] << rhs[2]
		;
}
inline std::ostream& operator<<(std::ostream &os, const btTransform &rhs)
{
	return os
		<< rhs.getBasis() << rhs.getOrigin();
}

namespace irr
{
namespace scene
{

using namespace polymesh;
typedef std::map<io::path, video::ITexture*> TEXTURE_MAP;

static u32 toByte(float v)
{
	return static_cast<u32>(255 * v);
}

// `óðì¬·é
// Ôèlð¾½êÅL ðÛ·é(delete·é)B
static irr::bullet::IShape* createRigidBody(const pmd::RigidBody &rb)
{
	switch(rb.rigidType)
	{
	case pmd::RIGID_SPHERE:
		return irr::bullet::CWorld::createSphereShape(rb.w);

	case pmd::RIGID_BOX:
		return irr::bullet::CWorld::createBoxShape(rb.w, rb.h, rb.d);

	case pmd::RIGID_CAPSULE:
		return irr::bullet::CWorld::createCapsuleShape(rb.w, rb.h);

	default:
		assert(false);
		return 0;
	}
}

// {[[JÀWàÅÌÌÊuð¾é
static btTransform getRigidTransformInBone(const pmd::RigidBody &rb)
{
	btMatrix3x3	btmRotationMat;
	btmRotationMat.setEulerZYX(
			rb.rotation.x,
			rb.rotation.y,
			rb.rotation.z );
	return btTransform(btmRotationMat,
			btVector3(
				rb.position.x,
				rb.position.y,
				rb.position.z ));
}

// {[Êuð¾é
static btVector3 getBonePosition(scene::IJoint *joint)
{
	if(joint){
		// {[Êuæ¾
		core::vector3df pos=joint->getOffset();
		/*
		std::cout
			<< joint->getName().c_str() << ','
			<<  pos.X << ',' << pos.Y << ',' << pos.Z
			<< std::endl
			;
			*/
		return btVector3(pos.X, pos.Y, pos.Z);
	}
	else{
		return btVector3(0, 0, 0);
	}
}

// Ìú»p[^
static btRigidBody::btRigidBodyConstructionInfo
createRigidBodyInfo(const pmd::RigidBody &rb, btCollisionShape *shape)
{
	// ¿Ê
	btScalar mass( 0.0f );
	if(rb.processType != pmd::PROCESS_SYNCBONE){
		// {[Ç]ÅÈ¢êÍ¿ÊðÝè
		mass = rb.weight;
	}

	// µ«e\
	btVector3 localInertia(0.0f, 0.0f ,0.0f);
	if(mass != 0.0f){
		shape->calculateLocalInertia(mass, localInertia);
	}

	// Ìp[^
	btRigidBody::btRigidBodyConstructionInfo info(
			mass, 0, shape, localInertia);
	// Ú®¸
	info.m_linearDamping  = rb.posAttenuation;
	// ñ]¸
	info.m_angularDamping = rb.rotAttenuation;
	// ½­Í
	info.m_restitution    = rb.recoile;
	// CÍ
	info.m_friction       = rb.friction;
	info.m_additionalDamping = true;

	return info;
}

// bulletÌconstraintð¶¬·éB
// Ôèlð¾½êÅL ðÛ·é(delete·é)B
static btGeneric6DofSpringConstraint *createConstraint(const pmd::Joint &c,
		btRigidBody* rigidBodyA, btRigidBody* rigidBodyB)
{
	// transform
	btMatrix3x3 rotation;
	rotation.setEulerZYX(c.rot.x, c.rot.y, c.rot.z);
	btTransform transform(rotation, btVector3(c.pos.x, c.pos.y, c.pos.z));

	// transform from rigidBodyA
	btTransform transformFromRigidBodyA =
		rigidBodyA->getWorldTransform().inverse() * transform;

	// transform from rigidBodyB
	btTransform transformFromRigidBodyB =
		rigidBodyB->getWorldTransform().inverse() * transform;

	// create constraint
	btGeneric6DofSpringConstraint *constraint =
		new btGeneric6DofSpringConstraint(
			*rigidBodyA, *rigidBodyB,
			transformFromRigidBodyA, transformFromRigidBodyB, true);

	// constraint position
	constraint->setLinearLowerLimit(
			btVector3(
				c.constraintPosA.x,
				c.constraintPosA.y,
				c.constraintPosA.z )
			);
	constraint->setLinearUpperLimit(
			btVector3(
				c.constraintPosB.x,
				c.constraintPosB.y,
				c.constraintPosB.z )
			);

	// constraint rotation
	constraint->setAngularLowerLimit(
			btVector3(
				c.constraintRotA.x,
				c.constraintRotA.y,
				c.constraintRotA.z )
			);
	constraint->setAngularUpperLimit(
			btVector3(
				c.constraintRotB.x,
				c.constraintRotB.y,
				c.constraintRotB.z ) );

	// 0 : translation X
	if( c.springPos.x != 0.0f )
	{
		constraint->enableSpring( 0, true );
		constraint->setStiffness( 0, c.springPos.x );
	}

	// 1 : translation Y
	if( c.springPos.y != 0.0f )
	{
		constraint->enableSpring( 1, true );
		constraint->setStiffness( 1, c.springPos.y );
	}

	// 2 : translation Z
	if( c.springPos.z != 0.0f )
	{
		constraint->enableSpring( 2, true );
		constraint->setStiffness( 2, c.springPos.z );
	}

	// 3 : rotation X
	// (3rd Euler rotational around new position of X axis,
	// range [-PI+epsilon, PI-epsilon] )
	constraint->enableSpring( 3, true );
	constraint->setStiffness( 3, c.springRot.x );

	// 4 : rotation Y
	// (2nd Euler rotational around new position of Y axis,
	// range [-PI/2+epsilon, PI/2-epsilon] )
	constraint->enableSpring( 4, true );
	constraint->setStiffness( 4, c.springRot.y );

	// 5 : rotation Z
	// (1st Euler rotational around Z axis,
	// range [-PI+epsilon, PI-epsilon] )
	constraint->enableSpring( 5, true );
	constraint->setStiffness( 5, c.springRot.z );

	return constraint;
}

static IAnimatedMesh* buildMesh(
		pmd::Loader &loader, video::IVideoDriver *driver)
{
	CCustomSkinnedMesh *mesh = new CCustomSkinnedMesh();

	////////////////////////////////////////////////////////////
	// store geometries
	////////////////////////////////////////////////////////////

	// vertices
	mesh->reallocate(loader.vertices.size());
	for(size_t i=0; i<loader.vertices.size(); ++i){
		pmd::Vertex &v=loader.vertices[i];
		mesh->addVertex(video::S3DVertex(
					v.pos.x, v.pos.y, v.pos.z,
					v.normal.x, v.normal.y, v.normal.z,
					video::SColor(255, 255, 255, 255), // white
					v.uv.x, v.uv.y),
				v.weight0*0.01f,  // [0, 100] to [0, 1.0]
				v.bone0, v.bone1
				);
	}
	mesh->recalculateBoundingBox();

	// triangles
	size_t index=0;
	TEXTURE_MAP texture_map;
	for(size_t i=0; i<loader.materials.size(); ++i){
		// each material has indexBuffer.
		pmd::Material &m=loader.materials[i];
		video::SColor diffuse(toByte(m.alpha),
				toByte(m.diffuse_color.x),
				toByte(m.diffuse_color.y),
				toByte(m.diffuse_color.z));
		video::SColor ambient(255,
				toByte(m.mirror_color.x),
				toByte(m.mirror_color.y),
				toByte(m.mirror_color.z));
		video::SColor specular(255,
				toByte(m.specular_color.x),
				toByte(m.specular_color.y),
				toByte(m.specular_color.z));
		// sharing vertexBuffer.
		SSharedMeshBuffer *indexBuffer=mesh->addIndexBuffer();
		// store triangle indices
		for(size_t j=0; j<m.vertex_count; ++j, ++index){
			indexBuffer->Indices.push_back(loader.indices[index]);
			// set vertex color
			mesh->setVertexColor(loader.indices[index], diffuse);
		}
		// setup material
		video::SMaterial &material=indexBuffer->Material;
		material.DiffuseColor=diffuse;
		material.AmbientColor=ambient;
		material.SpecularColor=specular;
		material.Shininess=m.specular;

		material.GouraudShading=false;
		material.Lighting=true;
		//material.BackfaceCulling=false;
		if(m.texture!=""){
			// setup texture
			io::path texture_path(
					util::cp932_to_fs(m.texture).c_str());
			texture_path.replace(L'\\', L'/');

#if 1
//			texture_path.replace(L'*', L'\0');

			int pos = texture_path.findFirst(L'*');
			io::path texture_path2;
			if (pos > 0) {
				texture_path2 = texture_path.subString(pos+1, texture_path.size() - pos - 1);
				texture_path = texture_path.subString(0, pos);
			}
#endif

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
# if 1
			video::ITexture *texture2 = 0;
			if (pos > 0) {
				texture2 = driver->getTexture(texture_path2);
				material.setTexture(1, texture2);
			}
#endif
			material.MaterialType=
				video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
		}
	}

	////////////////////////////////////////////////////////////
	// store joints
	////////////////////////////////////////////////////////////
	for(size_t i=0; i<loader.bones.size(); ++i){
		pmd::Bone &b=loader.bones[i];
		IJoint *joint=mesh->addJoint(b.name.c_str(),
				core::vector3df(b.pos.x, b.pos.y, b.pos.z));
	}
	// build herarchy
	for(size_t i=0; i<loader.bones.size(); ++i){
		pmd::Bone &b=loader.bones[i];
		IJoint *joint=mesh->getJoint(i);
		IJoint *parent=(b.parent_index==0xFFFF) ?
			mesh->getRootJoint() : mesh->getJoint(b.parent_index);
		parent->addChild(joint);
	}

	////////////////////////////////////////////////////////////
	// IK
	////////////////////////////////////////////////////////////
#if 1
    size_t listsize = loader.ik_list.size();
	for(size_t i=0; i<loader.ik_list.size(); ++i){
		pmd::IK &ik=loader.ik_list[i];
		CCustomSkinnedMesh::CIkResolver *ikResolver=mesh->addIkResolver(
				ik.index, ik.target, ik.length,
				ik.iterations, ik.weight
				);
	}
#endif
	////////////////////////////////////////////////////////////
	// store morphing
	////////////////////////////////////////////////////////////
	for(size_t i=0; i<loader.morph_list.size(); ++i){
		pmd::Morph &m=loader.morph_list[i];
		CCustomSkinnedMesh::CMorphing *morphing;
		if(m.type==pmd::MORPH_BASE){
			morphing=mesh->getMorphingBase();
		}
		else{
			morphing=mesh->addMorphing(m.name.c_str());
		}
		morphing->reallocate(m.indices.size());
		for(size_t j=0; j<m.indices.size(); ++j){
			morphing->addIndexCoord(m.indices[j],
					core::vector3df(
						m.pos_list[j].x, m.pos_list[j].y, m.pos_list[j].z));
		}
	}

	////////////////////////////////////////////////////////////
	// Bullet Physics
	////////////////////////////////////////////////////////////
	bullet::CWorld *bulletWorld=mesh->getBulletWorld();

	// store rigid bodies
	////////////////////////////////////////////////////////////
	for(size_t i=0; i<loader.rigids.size(); ++i){
		pmd::RigidBody &rb=loader.rigids[i];

		IJoint *joint=0;
		if(rb.boneIndex==0xffff){
			/*
			joint=mesh->getJoint(0);
			assert(joint->getName()=="Z^[");
			*/
		}
		else{
			joint=mesh->getJoint(rb.boneIndex);
		}
		//assert(joint);

		// bullet`óðì¬
		bullet::IShape* shape=createRigidBody(rb);
		assert(shape);

		// Ìp¨({[[JÀW)
		btTransform rigidOffsetInBone=getRigidTransformInBone(rb);

		// {[p¨
		btTransform boneTransform(
				btMatrix3x3::getIdentity(), getBonePosition(joint));

		// Ìp¨([hÀW)
		btTransform centerOfMass=boneTransform * rigidOffsetInBone;

		// Ìîñ
		btRigidBody::btRigidBodyConstructionInfo info
			=createRigidBodyInfo(rb, shape->getBulletShape());

		// Ìì¬
		bullet::IRigidBody *rigidBody=0;
		switch(rb.processType)
		{
		case 0 :
			// BoneÇ]
			rigidBody=bulletWorld->createKinematicsMoveRigidBody(
					rb.name, shape, info, centerOfMass, joint, rigidOffsetInBone);
			break;

		case 1 :
			// ¨Z
			rigidBody=bulletWorld->createPhysicsBoveRigidBody(
					rb.name, shape, info, centerOfMass,
					joint, rigidOffsetInBone);
			break;

		case 2 :
			// ¨Z(BoneÊuí¹)
			rigidBody=bulletWorld->
				createKinematicsMoveAndPhysicsRotateRigidBody(
						rb.name, shape, info, centerOfMass,
						joint, rigidOffsetInBone);
			break;

		}
		assert(rigidBody);
		bulletWorld->addRigidBody(rigidBody,
				0x0001 << rb.group, rb.target);
		rigidBody->drop();
		shape->drop();
	}

	// store constraints
	////////////////////////////////////////////////////////////
	for(size_t i=0; i<loader.joints.size(); ++i){
		pmd::Joint &c=loader.joints[i];
		bulletWorld->pushConstraint(createConstraint( c,
					bulletWorld->getRigidBody(c.rigidA)->getBulletRigidBody(),
					bulletWorld->getRigidBody(c.rigidB)->getBulletRigidBody())
				);
	}

	return mesh;
}

///////////////////////////////////////////////////////////////////////////////
// CPMDMeshFileLoader
///////////////////////////////////////////////////////////////////////////////
//! Constructor
CPMDMeshFileLoader::CPMDMeshFileLoader(ISceneManager *smgr)
	: SceneManager(smgr)
{
}

//! destructor
CPMDMeshFileLoader::~CPMDMeshFileLoader()
{
}

bool CPMDMeshFileLoader::isALoadableFileExtension(
		const io::path& filename) const
{
	return core::hasFileExtension ( filename, "pmd" );
}

//! creates/loads an animated mesh from the file.
//! \return Pointer to the created mesh. Returns 0 if loading failed.
//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
//! See IReferenceCounted::drop() for more information.
IAnimatedMesh* CPMDMeshFileLoader::createMesh(io::IReadFile* file)
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
	pmd::Loader loader;
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

