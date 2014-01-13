//#include "btJoint.h"
#include "btjoint.h"
#include "SRotPosKey.h"

namespace irr {

	static btTransform irrMatrix2btTransform(const core::matrix4 &m)
	{
		btTransform t;
		t.setFromOpenGLMatrix(m.getTransposed().pointer());
		return t;
	}

namespace scene {

btJoint::btJoint(const core::stringc& name, const core::vector3df &pos)
	:
	Name(name), Offset(pos.X, pos.Y, pos.Z), RelativeOffset(0, 0, 0),
	Parent(0), Curve(0), IsConstrainted(false)
	{
		Motion.setIdentity();
		Accumulation.setIdentity();
		Skinning.setIdentity();
	}

void btJoint::addChild(IJoint *child)
{
	child->grab();
	Children.push_back(child);
	child->setParent(this);
	child->setRelativeOffset(child->getOffset()-getOffset());
}

void btJoint::update()
{
	Accumulation=Motion;
	if(Parent){
		Accumulation*=irrMatrix2btTransform(Parent->getAccumulation());
	}
}

void btJoint::updateRecursive()
{
	update();
	for(size_t i=0; i<Children.size(); ++i){
		Children[i]->updateRecursive();
	}
}

void btJoint::calcSkinning()
{
	// ToDo: optimize
	btTransform offset;
	offset.setOrigin(-Offset);
	Skinning=offset * Accumulation;
}

void btJoint::setFrame(s32 frame)
{
	if(Curve){
		SRotPosKey motion=Curve->getKey(frame);
		btVector3 pos(motion.Position.X, motion.Position.Y, motion.Position.Z);
		Motion.setOrigin(pos);
		btQuaternion rot(
				motion.Rotation.X, motion.Rotation.Y, motion.Rotation.Z,
				motion.Rotation.W);
		Motion.setRotation(rot);
	}
	else{
		Motion.setIdentity();
	}
}

core::matrix4 btJoint::getSkinning()const
{
	core::matrix4 m;
	float m16[16];
	Skinning.getOpenGLMatrix(m16);
	m[0]=m16[0]; m[1]=m16[1]; m[2]=m16[2]; m[3]=m16[3];
	m[4]=m16[4]; m[5]=m16[5]; m[6]=m16[6]; m[7]=m16[7];
	m[8]=m16[8]; m[9]=m16[9]; m[10]=m16[10]; m[11]=m16[11];
	m[12]=m16[12]; m[13]=m16[13]; m[14]=m16[14]; m[15]=m16[15];
	return m.getTransposed();
}

core::vector3df btJoint::getOffset()const
{
	return core::vector3df(Offset.x(), Offset.y(), Offset.z());
}

void btJoint::setRelativeOffset(const core::vector3df &offset)
{
	RelativeOffset.setX(offset.X);
	RelativeOffset.setY(offset.Y);
	RelativeOffset.setZ(offset.Z);
}

core::matrix4 btJoint::getAccumulation()const
{
	core::matrix4 m;
	float m16[16];
	Accumulation.getOpenGLMatrix(m16);
	m[0]=m16[0]; m[1]=m16[1]; m[2]=m16[2]; m[3]=m16[3];
	m[4]=m16[4]; m[5]=m16[5]; m[6]=m16[6]; m[7]=m16[7];
	m[8]=m16[8]; m[9]=m16[9]; m[10]=m16[10]; m[11]=m16[11];
	m[12]=m16[12]; m[13]=m16[13]; m[14]=m16[14]; m[15]=m16[15];
	return m.getTransposed();
}

core::vector3df btJoint::getAccumulatedPosition()const
{
	const btVector3 &p=Accumulation.getOrigin();
	return core::vector3df(p.x(), p.y(), p.z());
}

core::quaternion btJoint::getCurrentRotation()const
{
	const btQuaternion &q=Motion.getRotation();
	return core::quaternion(q.x(), q.y(), q.z(), q.w());
}

void btJoint::setCurrentRotation(const core::quaternion &r)
{
	Motion.setRotation(btQuaternion(r.X, r.Y, r.Z, r.W));
}

} // namespace scene
} // namespace irr
