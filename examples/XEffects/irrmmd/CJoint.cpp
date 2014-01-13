#include "CJoint.h"
#include "SRotPosKey.h"

namespace irr {

// fix core::quaternion.getMatrix
static core::matrix4 getMatrix(const core::quaternion &rot)
{
	float xx=rot.X*rot.X;
	float yy=rot.Y*rot.Y;
	float zz=rot.Z*rot.Z;

	float xy=rot.X*rot.Y;
	float yz=rot.Y*rot.Z;
	float zx=rot.Z*rot.X;

	float wx=rot.W*rot.X;
	float wy=rot.W*rot.Y;
	float wz=rot.W*rot.Z;

	core::matrix4 m(core::matrix4::EM4CONST_IDENTITY);
	
	m[0]=1-2*(yy+zz);
	m[1]=2*(xy+wz);
	m[2]=2*(zx-wy);
	m[4]=2*(xy-wz);
	m[5]=1-2*(xx+zz);
	m[6]=2*(yz+wx);
	m[8]=2*(zx+wy);
	m[9]=2*(yz-wx);
	m[10]=1-2*(xx+yy);

	return m;
}

namespace scene {

CJoint::CJoint(const core::stringc& name, const core::vector3df &pos)
: Name(name), 
	Offset(pos), RelativeOffset(0, 0, 0),
	Accumulation(core::matrix4::EM4CONST_IDENTITY),
	Skinning(core::matrix4::EM4CONST_IDENTITY),
	MotionRotation(0, 0, 0, 1), MotionTranslation(0, 0, 0),
	Parent(0), Curve(0), IsConstrainted(false)
{
}

void CJoint::addChild(IJoint *child)
{
	child->grab();
	Children.push_back(child);
	child->setParent(this);
	child->setRelativeOffset(child->getOffset()-Offset);
}

void CJoint::setFrame(s32 frame)
{
	if(Curve){
		SRotPosKey motion=Curve->getKey(frame);
		MotionTranslation=motion.Position;
		MotionRotation=motion.Rotation;
	}
	else{
		MotionTranslation=core::vector3df(0, 0, 0);
		MotionRotation=core::quaternion(0.0f, 0.0f, 0.0f, 1.0f);
	}
}

void CJoint::updateRecursive()
{
	update();
	for(size_t i=0; i<Children.size(); ++i){
		Children[i]->updateRecursive();
	}
}

void CJoint::update()
{
	// Accumulation=Parent->Accumulation * translation * rotation

	// translation
	core::matrix4 translation(core::matrix4::EM4CONST_IDENTITY);
	translation.setTranslation(MotionTranslation+RelativeOffset);

	if(Parent){
		Accumulation=
			//Parent->Accumulation * translation * MotionRotation.getMatrix();
			Parent->getAccumulation() * translation * getMatrix(MotionRotation);
	}
	else{
		//Accumulation=translation * MotionRotation.getMatrix();
		Accumulation=translation * getMatrix(MotionRotation);
	}
}

void CJoint::calcSkinning()
{
	// ToDo: optimize
	core::matrix4 offset(core::matrix4::EM4CONST_IDENTITY);
	offset.setTranslation(-Offset);
	Skinning=Accumulation*offset;
}

} // scene
} // irr
