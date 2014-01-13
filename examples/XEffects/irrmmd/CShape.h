#ifndef CLASS_SHAPE_H_INCLUDED
#define CLASS_SHAPE_H_INCLUDED

#include "IShape.h"

class btStaticPlaneShape;
class btSphereShape;
class btBoxShape;
class btCapsuleShape;

namespace irr {
namespace bullet {

///////////////////////////////////////////////////////////////////////////////
class CBoxShape : public IShape
{
public:
	CBoxShape(float w, float h, float d);
	~CBoxShape();
	virtual btCollisionShape* getBulletShape();
	virtual void draw();

private:
	btBoxShape *Shape;
};

///////////////////////////////////////////////////////////////////////////////
class CSphereShape : public IShape
{
public:
	CSphereShape(float radius);
	~CSphereShape();
	virtual btCollisionShape* getBulletShape();
	virtual void draw();

private:
	btSphereShape *Shape;
};

///////////////////////////////////////////////////////////////////////////////
class CCapsuleShape : public IShape
{
public:
	CCapsuleShape(float w, float h);
	~CCapsuleShape();
	virtual btCollisionShape* getBulletShape();
	virtual void draw();

private:
	btCapsuleShape *Shape;
};

///////////////////////////////////////////////////////////////////////////////
class CPlaneShape : public IShape
{
public:
	CPlaneShape();
	~CPlaneShape();
	virtual btCollisionShape* getBulletShape();
	virtual void draw();

private:
	btStaticPlaneShape *Shape;
};

} // namespace bullet
} // namespace irr

#endif // CLASS_SHAPE_H_INCLUDED
