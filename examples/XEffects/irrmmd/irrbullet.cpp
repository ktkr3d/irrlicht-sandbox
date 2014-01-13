#include "irrbullet.h"
#include "CJoint.h"
#include "CShape.h"
#include "CRigidBody.h"

static const float WORLD_END=100.0f;

namespace irr {
namespace bullet {

CWorld::CWorld() :
	CollisionConfiguration(0),
	CollisionDispatcher(0),
	Sweep(0),
	ConstraintSolver(0),
	World(0)
{
	CollisionConfiguration=new btDefaultCollisionConfiguration;
	CollisionDispatcher=new btCollisionDispatcher(CollisionConfiguration);
	Sweep=new btAxisSweep3(
			btVector3(-WORLD_END, -WORLD_END, -WORLD_END),
			btVector3(WORLD_END, WORLD_END, WORLD_END)
			);
	ConstraintSolver=new btSequentialImpulseConstraintSolver;

	World=new btDiscreteDynamicsWorld(
			CollisionDispatcher, Sweep,
			ConstraintSolver, CollisionConfiguration);

//	World->setGravity(btVector3(0, -9.8f * 2.5f, 0));
	World->setGravity(btVector3(0, -9.8f, 0));
}

CWorld::~CWorld()
{
	// clear constraints
	for(u32 i=0; i<Constraints.size(); ++i){
		World->removeConstraint(Constraints[i]);
		delete Constraints[i];
	}
	Constraints.clear();

	// clear rigid bodies
	for(u32 i=0; i<RigidBodies.size(); ++i){
		World->removeRigidBody(RigidBodies[i]->getBulletRigidBody());
		RigidBodies[i]->drop();
	}
	RigidBodies.clear();

	// clear world
	delete World;
	World=0;
	delete ConstraintSolver;
	ConstraintSolver=0;
	delete Sweep;
	Sweep=0;
	delete CollisionDispatcher;
	CollisionDispatcher=0;
	delete CollisionConfiguration;
	CollisionConfiguration=0;
}

void
CWorld::pushConstraint(btGeneric6DofSpringConstraint *constraint)
{
	Constraints.push_back(constraint);
	World->addConstraint(constraint);
}

void CWorld::update(u32 msec)
{
	World->stepSimulation(static_cast<btScalar>(msec), 1, msec * 0.001f);
}

void CWorld::syncBone()
{
	for(u32 i=0; i<RigidBodies.size(); ++i){
		RigidBodies[i]->syncBone();
	}
}

IRigidBody *CWorld::createKinematicsMoveRigidBody(
		const std::string &name, IShape *shape,
		btRigidBody::btRigidBodyConstructionInfo &info,
		const btTransform &centerOfMass,
		scene::IJoint *bone, const btTransform &offsetInBone)
{
	return new CKinematicsRigidBody(name, shape, info, centerOfMass, bone, offsetInBone);
}

IRigidBody *CWorld::createPhysicsBoveRigidBody(
		const std::string &name, IShape *shape,
		btRigidBody::btRigidBodyConstructionInfo &info,
		const btTransform &centerOfMass,
		scene::IJoint *bone, const btTransform &offsetInBone
		)
{
	return new CPhysicsBoneRigidBody(name,
				shape, info, centerOfMass, bone, offsetInBone);
}

IRigidBody *CWorld::createKinematicsMoveAndPhysicsRotateRigidBody(
		const std::string &name, IShape *shape,
		btRigidBody::btRigidBodyConstructionInfo &info,
		const btTransform &centerOfMass,
		scene::IJoint *bone, const btTransform &offsetInBone
		)
{
	return new CKinematicsMoveAndPhysicsRotateRigidBody(name,
				shape, info, centerOfMass, bone, offsetInBone);
}

void
CWorld::addRigidBody(IRigidBody *rigidBody, short group, short mask)
{
	rigidBody->grab();
	RigidBodies.push_back(rigidBody);
	World->addRigidBody(rigidBody->getBulletRigidBody(), group, mask);
}

///////////////////////////////////////////////////////////////////////////////
// static
///////////////////////////////////////////////////////////////////////////////
IShape *CWorld::createPlaneShape()
{
	return new CPlaneShape;
}

IShape *CWorld::createBoxShape(float w, float h, float d)
{
	return new CBoxShape(w, h, d);
}

IShape *CWorld::createSphereShape(float radius)
{
	return new CSphereShape(radius);
}

IShape *CWorld::createCapsuleShape(float w, float h)
{
	return new CCapsuleShape(w, h);
}


} // namespace bullet
} // namespace irr
