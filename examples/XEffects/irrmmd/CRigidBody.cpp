#include "CRigidBody.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning( disable : 4819 )
#endif
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

//#include <windows.h>
#include <GL/gl.h>

#include "IJoint.h"
#include "IShape.h"

namespace irr {

static core::matrix4 getMatrix(const btTransform &t)
{
	core::matrix4 m;

	float m16[16];
	t.getOpenGLMatrix(m16);
	m[0]=m16[0]; m[1]=m16[1]; m[2]=m16[2]; m[3]=m16[3];
	m[4]=m16[4]; m[5]=m16[5]; m[6]=m16[6]; m[7]=m16[7];
	m[8]=m16[8]; m[9]=m16[9]; m[10]=m16[10]; m[11]=m16[11];
	m[12]=m16[12]; m[13]=m16[13]; m[14]=m16[14]; m[15]=m16[15];

	return m;
}

static btTransform getTransform(const core::matrix4 &m)
{
	btTransform t;
	t.setFromOpenGLMatrix(m.pointer());
	return t;
}

namespace bullet {

///////////////////////////////////////////////////////////////////////////////
// IRigidBody
///////////////////////////////////////////////////////////////////////////////
void IRigidBody::draw()
{
	glPushMatrix();
	glDisable(GL_LIGHTING);
	btScalar m[16];
	getBulletRigidBody()->getCenterOfMassTransform().getOpenGLMatrix(m);
	glMultMatrixf( m );
	getShape()->draw();
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// CDefaultRigidBody
///////////////////////////////////////////////////////////////////////////////
CDefaultRigidBody::CDefaultRigidBody(const std::string &name, IShape *shape,
		btRigidBody::btRigidBodyConstructionInfo &info,
		const btTransform &centerOfMass)
: Name(name)
{
	shape->grab();
	Shape=shape;

	MotionState=new btDefaultMotionState(centerOfMass);
	info.m_motionState=MotionState;

	RigidBody=new btRigidBody(info);
}

CDefaultRigidBody::~CDefaultRigidBody()
{
	delete RigidBody;
	RigidBody=0;

	delete MotionState;
	MotionState=0;

	Shape->drop();
	Shape=0;
}

void CDefaultRigidBody::draw()
{
	glColor3f(0.0f, 0.0f, 0.0f);
	IRigidBody::draw();
}

///////////////////////////////////////////////////////////////////////////////
// CKinematicsRigidBody
///////////////////////////////////////////////////////////////////////////////
CKinematicsRigidBody::CKinematicsRigidBody(const std::string &name, IShape *shape,
		btRigidBody::btRigidBodyConstructionInfo &info,
		const btTransform &centerOfMass,
		scene::IJoint *bone, const btTransform &offsetInBone)
: Name(name), StartPosition(centerOfMass), Bone(bone), OffsetInBone(offsetInBone), isInitialized(false)
{
	shape->grab();
	Shape=shape;

	info.m_motionState=this;

	RigidBody = new btRigidBody(info);
	RigidBody->setSleepingThresholds( 0.0f, 0.0f );

	// Kinematic
	RigidBody->setCollisionFlags(RigidBody->getCollisionFlags() |
			btCollisionObject::CF_KINEMATIC_OBJECT );
	RigidBody->setActivationState( DISABLE_DEACTIVATION );
}

CKinematicsRigidBody::~CKinematicsRigidBody()
{
	delete RigidBody;
	RigidBody=0;
}

btTransform CKinematicsRigidBody::getBoneRigidTransform()const
{
	return getTransform(Bone->getAccumulation()) * OffsetInBone;
}

/// synchronizes world transform from user to physics
void CKinematicsRigidBody::getWorldTransform(
		btTransform& centerOfMassWorldTrans) const
{
	if(isInitialized){
		centerOfMassWorldTrans=getBoneRigidTransform();
	}
	else{
		// todo
		centerOfMassWorldTrans=StartPosition;
		isInitialized=true;
	}
}

/// synchronizes world transform from physics to user
/// Bullet only calls the update of worldtransform for active objects
void CKinematicsRigidBody::setWorldTransform(
		const btTransform& centerOfMassWorldTrans )
{
	// do nothing
}

void CKinematicsRigidBody::draw()
{
	if(Bone){
		glColor3f(1.0f, 0.0f, 0.0f);
	}
	else{
		glColor3f(0.5f, 0.0f, 0.0f);
	}
	IRigidBody::draw();
}

///////////////////////////////////////////////////////////////////////////////
// CPhysicsBoneRigidBody
///////////////////////////////////////////////////////////////////////////////
CPhysicsBoneRigidBody::CPhysicsBoneRigidBody(
		const std::string &name, IShape *shape,
			btRigidBody::btRigidBodyConstructionInfo &info,
			const btTransform &centerOfMass,
			scene::IJoint *bone,
			const btTransform &offsetInBone
			)
: Name(name), Bone(bone), OffsetInBoneInv(offsetInBone.inverse()),
	m_graphicsWorldTrans(centerOfMass)
{
	shape->grab();
	Shape=shape;

	info.m_motionState=this;

	RigidBody = new btRigidBody(info);
	RigidBody->setSleepingThresholds( 0.0f, 0.0f );
}

CPhysicsBoneRigidBody::~CPhysicsBoneRigidBody()
{
	delete RigidBody;
	RigidBody=0;
}

void CPhysicsBoneRigidBody::getWorldTransform(
		btTransform& centerOfMassWorldTrans ) const
{
	centerOfMassWorldTrans = m_graphicsWorldTrans;
}

void CPhysicsBoneRigidBody::setWorldTransform(
		const btTransform& centerOfMassWorldTrans)
{
	m_graphicsWorldTrans = centerOfMassWorldTrans ;

	// update bone
	if(Bone){
		Bone->setAccumulation(
				getMatrix(m_graphicsWorldTrans * OffsetInBoneInv));
	}
}

void CPhysicsBoneRigidBody::draw()
{
	if(Bone){
		glColor3f(0.0f, 1.0f, 0.0f);
	}
	else{
		glColor3f(0.0f, 0.5f, 0.0f);
	}
	IRigidBody::draw();
}

void CPhysicsBoneRigidBody::syncBone()
{
	core::vector3df pos=Bone->getAccumulatedPosition();
	RigidBody->setCenterOfMassTransform(btTransform(
				btMatrix3x3::getIdentity(), btVector3(pos.X, pos.Y, pos.Z)));
	// ’âŽ~‚³‚¹‚é
	RigidBody->setLinearVelocity(
			btVector3(0.0f, 0.0f, 0.0f));
	RigidBody->setAngularVelocity(
			btVector3(0.0f, 0.0f, 0.0f));
	RigidBody->setInterpolationLinearVelocity(
			btVector3(0.0f, 0.0f, 0.0f));
	RigidBody->setInterpolationAngularVelocity(
			btVector3(0.0f, 0.0f, 0.0f));
	RigidBody->setInterpolationWorldTransform(
			RigidBody->getCenterOfMassTransform());
	RigidBody->clearForces();
}

///////////////////////////////////////////////////////////////////////////////
// CKinematicsMoveAndPhysicsRotateRigidBody
///////////////////////////////////////////////////////////////////////////////
CKinematicsMoveAndPhysicsRotateRigidBody::CKinematicsMoveAndPhysicsRotateRigidBody(
		const std::string &name, IShape *shape,
			btRigidBody::btRigidBodyConstructionInfo &info,
			const btTransform &centerOfMass,
			scene::IJoint *bone,
			const btTransform &offsetInBone
			)
: Name(name), Bone(bone), m_BoneOffsetInv(offsetInBone.inverse()),
	m_graphicsWorldTrans(centerOfMass)
{
	shape->grab();
	Shape=shape;

	info.m_motionState=this;

	RigidBody = new btRigidBody(info);
	RigidBody->setSleepingThresholds( 0.0f, 0.0f );
}

CKinematicsMoveAndPhysicsRotateRigidBody::~CKinematicsMoveAndPhysicsRotateRigidBody()
{
	delete RigidBody;
	RigidBody=0;
}

void CKinematicsMoveAndPhysicsRotateRigidBody::getWorldTransform(
		btTransform& centerOfMassWorldTrans ) const
{
	centerOfMassWorldTrans = m_graphicsWorldTrans;
}

void CKinematicsMoveAndPhysicsRotateRigidBody::setWorldTransform(
		const btTransform& centerOfMassWorldTrans)
{
	m_graphicsWorldTrans = centerOfMassWorldTrans ;

	if(Bone){
		btTransform t=m_graphicsWorldTrans * m_BoneOffsetInv;

		/*
		core::vector3df v=Bone->getAccumulatedPosition();
		btVector3 boneOffset(v.X, v.Y, v.Z);

		// update rigid
		//RigidBody->translate(boneOffset-t.getOrigin());

		// upate bone
		t.setOrigin(boneOffset);
		*/
		Bone->setAccumulation(getMatrix(t));
	}
}

void CKinematicsMoveAndPhysicsRotateRigidBody::draw()
{
	if(Bone){
		glColor3f(1.0f, 1.0f, 0.0f);
	}
	else{
		glColor3f(0.5f, 0.5f, 0.0f);
	}
	IRigidBody::draw();
}

void CKinematicsMoveAndPhysicsRotateRigidBody::syncBone()
{
	if(!Bone){
		return;
	}

	core::vector3df pos=Bone->getAccumulatedPosition();
	RigidBody->setCenterOfMassTransform(getTransform(Bone->getAccumulation()));

	// ’âŽ~‚³‚¹‚é
	RigidBody->setLinearVelocity(
			btVector3(0.0f, 0.0f, 0.0f));
	RigidBody->setAngularVelocity(
			btVector3(0.0f, 0.0f, 0.0f));
	RigidBody->setInterpolationLinearVelocity(
			btVector3(0.0f, 0.0f, 0.0f));
	RigidBody->setInterpolationAngularVelocity(
			btVector3(0.0f, 0.0f, 0.0f));
	RigidBody->setInterpolationWorldTransform(
			RigidBody->getCenterOfMassTransform());
	RigidBody->clearForces();
}

} // namespace
} // namespace

