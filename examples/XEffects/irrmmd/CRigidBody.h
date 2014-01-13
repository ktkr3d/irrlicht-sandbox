#ifndef CLASS_RIGID_BODY_H_INCLUDED
#define CLASS_RIGID_BODY_H_INCLUDED

#include "IRigidBody.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning( disable : 4819 )
#endif
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif 

#include <string>

namespace irr {

namespace scene {
class IJoint;
}

namespace bullet {

class IShape;

//! ボーンと連動しない剛体。
class CDefaultRigidBody : public IRigidBody
{
public:
	CDefaultRigidBody(const std::string &name, IShape *shape, 
			btRigidBody::btRigidBodyConstructionInfo &info,
			const btTransform &centerOfMass);
	virtual ~CDefaultRigidBody();
	virtual btRigidBody* getBulletRigidBody()const{ return RigidBody; }
	virtual IShape* getShape()const{ return Shape; }
	virtual void draw();
	virtual std::string getName()const{ return Name; }

private:
	std::string Name;
	IShape *Shape;
	btDefaultMotionState *MotionState;
	btRigidBody *RigidBody;
};


//! ボーンに連動する剛体。
class CKinematicsRigidBody : public IRigidBody, btMotionState
{
public:
	CKinematicsRigidBody(const std::string &name, IShape *shape,
			btRigidBody::btRigidBodyConstructionInfo &info,
			const btTransform &centerOfMass,
			scene::IJoint *bone,
			const btTransform &offsetInBone
			);
	virtual ~CKinematicsRigidBody();
	virtual btRigidBody* getBulletRigidBody()const{ return RigidBody; }
	virtual IShape* getShape()const{ return Shape; }
	virtual void draw();
	virtual std::string getName()const{ return Name; }

private:
	std::string Name;
	btTransform getBoneRigidTransform()const;
	/// synchronizes world transform from user to physics
	virtual void getWorldTransform(
			btTransform& centerOfMassWorldTrans) const;
	/// synchronizes world transform from physics to user
	/// Bullet only calls the update of worldtransform for active objects
	virtual void setWorldTransform( 
			const btTransform& centerOfMassWorldTrans );

	mutable bool isInitialized;
	IShape *Shape;
	btRigidBody *RigidBody;
	btTransform StartPosition;
	btTransform OffsetInBone;
	scene::IJoint *Bone;
};


//! 物理演算の結果をボーンに反映する剛体。
class CPhysicsBoneRigidBody : public IRigidBody, btMotionState
{
public:
	CPhysicsBoneRigidBody(const std::string &name, IShape *shape,
			btRigidBody::btRigidBodyConstructionInfo &info,
			const btTransform &centerOfMass,
			scene::IJoint *bone,
			const btTransform &offsetInBone
			);
	virtual ~CPhysicsBoneRigidBody();
	virtual btRigidBody* getBulletRigidBody()const{ return RigidBody; }
	virtual IShape* getShape()const{ return Shape; }
	virtual void draw();
	virtual std::string getName()const{ return Name; }
	virtual void syncBone();

private:
	virtual void getWorldTransform(
			btTransform& centerOfMassWorldTrans ) const ;
	

	virtual void setWorldTransform(
			const btTransform& centerOfMassWorldTrans);

	std::string Name;
	IShape *Shape;
	btRigidBody *RigidBody;
	scene::IJoint *Bone;
	btTransform OffsetInBoneInv;
	btTransform m_graphicsWorldTrans;
};


//! 位置はボーンに連動し、回転は物理演算を反映する剛体。
class CKinematicsMoveAndPhysicsRotateRigidBody : public IRigidBody, btMotionState
{
public:
	CKinematicsMoveAndPhysicsRotateRigidBody(const std::string &name, 
			IShape *shape,
			btRigidBody::btRigidBodyConstructionInfo &info,
			const btTransform &centerOfMass,
			scene::IJoint *bone,
			const btTransform &offsetInBone
			);
	virtual ~CKinematicsMoveAndPhysicsRotateRigidBody();
	virtual btRigidBody* getBulletRigidBody()const{ return RigidBody; }
	virtual IShape* getShape()const{ return Shape; }
	virtual void draw();
	virtual std::string getName()const{ return Name; }
	virtual void syncBone();

private:
	std::string Name;
	virtual void getWorldTransform(
			btTransform& centerOfMassWorldTrans ) const ;

	virtual void setWorldTransform(
			const btTransform& centerOfMassWorldTrans);

	IShape *Shape;
	btTransform m_graphicsWorldTrans;
	scene::IJoint *Bone;
	btTransform m_BoneOffsetInv;
	btRigidBody *RigidBody;
};


} // namespace bullet
} // namespace irr

#endif // CLASS_RIGID_BODY_H_INCLUDED
