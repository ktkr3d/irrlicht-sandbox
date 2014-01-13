#ifndef BULLET_H_INCLUDED
#define BULLET_H_INCLUDED

/**
 * Irrlichtå¸ÇØbullet wrapper
 */

#include <irrlicht.h>

// VCÇÃåxçêâÒî
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
	class IRigidBody;

	class CWorld : public IReferenceCounted
	{
	public:
		CWorld();
		~CWorld();

		IRigidBody* getRigidBody(u32 index){ return RigidBodies[index]; }

		void pushConstraint(btGeneric6DofSpringConstraint *constraint);

		void update(u32 msec);
		void syncBone();

		IRigidBody *createKinematicsMoveRigidBody(
				const std::string &name, IShape *shape,
				btRigidBody::btRigidBodyConstructionInfo &info,
				const btTransform &centerOfMass,
				scene::IJoint *bone, const btTransform &offsetInBone);
		IRigidBody *createPhysicsBoveRigidBody(
				const std::string &name, IShape *shape,
				btRigidBody::btRigidBodyConstructionInfo &info,
				const btTransform &centerOfMass,
				scene::IJoint *bone, const btTransform &offsetInBone
				);
		IRigidBody *createKinematicsMoveAndPhysicsRotateRigidBody(
				const std::string &name, IShape *shape,
				btRigidBody::btRigidBodyConstructionInfo &info,
				const btTransform &centerOfMass,
				scene::IJoint *bone, const btTransform &offsetInBone
				);
		
		void 
			addRigidBody(IRigidBody *rigidBody, short group, short mask);

		static IShape *createPlaneShape();
		static IShape *createBoxShape(float w, float h, float d);
		static IShape *createSphereShape(float radius);
		static IShape *createCapsuleShape(float w, float h);

	private:
		// world variables
		btDefaultCollisionConfiguration *CollisionConfiguration;
		btCollisionDispatcher *CollisionDispatcher;
		btAxisSweep3 *Sweep;
		btSequentialImpulseConstraintSolver *ConstraintSolver;
		btDiscreteDynamicsWorld *World;

		core::array<IRigidBody*> RigidBodies;
		core::array<btGeneric6DofSpringConstraint*> Constraints;
	};

} // namespace bullet
} // namespace irr

#endif  // BULLET_H_INCLUDED
