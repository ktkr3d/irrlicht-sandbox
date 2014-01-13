#ifndef CLASS_JOINT_H_INCLUDED
#define CLASS_JOINT_H_INCLUDED

#include "IJoint.h"

namespace irr {
namespace scene { 

class CJoint : public IJoint
{
	IJoint *Parent;
	core::array<IJoint*> Children;
	core::stringc Name;

	SRotPosLinearCurve *Curve;

	// position from origin
	core::vector3df Offset;
	// relative from parent
	core::vector3df RelativeOffset;
	// current frame status
	core::vector3df MotionTranslation;
	core::quaternion MotionRotation;
	// skinning matrix
	core::matrix4 Accumulation;
	core::matrix4 Skinning;
	// IK param
	bool IsConstrainted;

public:
	CJoint(const core::stringc& name, const core::vector3df &pos);

	virtual void addChild(IJoint *child);
	virtual void setParent(IJoint *parent){ Parent=parent; }
	virtual IJoint* getParent()const{ return Parent; }
	virtual void update();
	virtual void updateRecursive();
	virtual void calcSkinning();
	virtual void assignCurve(SRotPosLinearCurve *curve){ Curve=curve; }
	virtual void setFrame(s32 frame);
	virtual core::stringc getName()const{ return Name; }
	virtual core::matrix4 getSkinning()const{ return Skinning; }
	virtual core::vector3df getOffset()const{ return Offset; }
	virtual void setRelativeOffset(const core::vector3df &offset){ 
		RelativeOffset=offset; 
	}
	virtual core::matrix4 getAccumulation()const { return Accumulation; }
	virtual void setAccumulation(const core::matrix4 &m){ Accumulation=m; }
	virtual core::vector3df getAccumulatedPosition()const{ 
		return Accumulation.getTranslation(); 
	}
	virtual core::quaternion getCurrentRotation()const{ 
		return MotionRotation;
	}
	virtual void setCurrentRotation(const core::quaternion &r){ 
		MotionRotation=r;
	}
	virtual void setConstraint(bool enable){ IsConstrainted=enable; }
	virtual bool getConstraint(){ return IsConstrainted; } 
};

} // namespace scene
} // namespace irr

#endif // CLASS_JOINT_H_INCLUDED
