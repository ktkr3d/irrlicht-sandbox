#include "CJoint.h"

// VCÇÃåxçêâÒî
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning( disable : 4819 )
#endif
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif 

namespace irr {
namespace scene {

class btJoint : public IJoint
{
	IJoint *Parent;
	core::array<IJoint*> Children;
	core::stringc Name;

	SRotPosLinearCurve *Curve;

	// position from origin
	btVector3 Offset;
	// relative from parent
	btVector3 RelativeOffset;
	// current frame status
	btTransform Motion;
	// skinning matrix
	btTransform Accumulation;
	btTransform Skinning;
	// IK param
	bool IsConstrainted;

public:
	btJoint(const core::stringc& name, const core::vector3df &pos);

	virtual void addChild(IJoint *child);
	virtual void setParent(IJoint *parent){ Parent=parent; }
	virtual IJoint* getParent()const{ return Parent; }
	virtual void update();
	virtual void updateRecursive();
	virtual void calcSkinning();
	virtual void assignCurve(SRotPosLinearCurve *curve){ Curve=curve; }
	virtual void setFrame(s32 frame);
	virtual core::stringc getName()const{ return Name; }
	virtual core::matrix4 getSkinning()const;
	virtual core::vector3df getOffset()const;
	virtual void setRelativeOffset(const core::vector3df &offset);
	virtual core::matrix4 getAccumulation()const;
	virtual core::vector3df getAccumulatedPosition()const;
	virtual core::quaternion getCurrentRotation()const;
	virtual void setCurrentRotation(const core::quaternion &r);
	virtual void setConstraint(bool enable){ IsConstrainted=enable; }
	virtual bool getConstraint(){ return IsConstrainted; } 
};

} // namespace scene
} // namespace irr
