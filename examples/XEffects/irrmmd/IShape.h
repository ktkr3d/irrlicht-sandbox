#ifndef INTERFACE_SHAPE_H_INCLUDED
#define INTERFACE_SHAPE_H_INCLUDED

#include <irrlicht.h>

class btCollisionShape;

namespace irr {
namespace bullet {

class IShape : public IReferenceCounted
{
public:
	virtual ~IShape(){}
	virtual btCollisionShape* getBulletShape()=0;
	virtual void draw()=0;
};

} // namespace bullet
} // namespace irr

#endif // INTERFACE_SHAPE_H_INCLUDED
