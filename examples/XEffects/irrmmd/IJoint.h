#ifndef INTERFACE_JOINT_H_INCLUDED
#define INTERFACE_JOINT_H_INCLUDED

#include <irrlicht.h>
#include "SCurve.h"

namespace irr {
namespace scene { 

class IJoint : public IReferenceCounted
{
public:
	virtual ~IJoint(){}

	virtual void addChild(IJoint *child)=0;
	virtual void setParent(IJoint *parent)=0;
	virtual IJoint* getParent()const=0;
	virtual void update()=0;
	virtual void updateRecursive()=0;
	virtual void calcSkinning()=0;
	virtual void assignCurve(SRotPosLinearCurve *curve)=0;
	virtual void setFrame(s32 frame)=0;
	virtual core::stringc getName()const=0;
	virtual core::matrix4 getSkinning()const=0;
	virtual core::vector3df getOffset()const=0;
	virtual void setRelativeOffset(const core::vector3df &offset)=0;
	virtual core::matrix4 getAccumulation()const=0;
	virtual void setAccumulation(const core::matrix4 &m)=0;
	virtual core::vector3df getAccumulatedPosition()const=0;
	virtual core::quaternion getCurrentRotation()const=0;
	virtual void setCurrentRotation(const core::quaternion &r)=0;
	virtual void setConstraint(bool enable)=0;
	virtual bool getConstraint()=0;
};

} // namespace
} // namespace

#endif // INTERFACE_JOINT_H_INCLUDED
