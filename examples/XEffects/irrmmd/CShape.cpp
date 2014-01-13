#include "CShape.h"
//#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning( disable : 4819 )
#endif
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace irr {
namespace bullet {

///////////////////////////////////////////////////////////////////////////////
// CBoxShape
///////////////////////////////////////////////////////////////////////////////
CBoxShape::CBoxShape(float w, float h, float d)
{
	Shape=new btBoxShape(btVector3(w, h, d));
}

CBoxShape::~CBoxShape()
{
	delete Shape;
	Shape=0;
}

void CBoxShape::draw()
{
	glPushMatrix();
	btVector3 halfExtent = Shape->getHalfExtentsWithMargin();
	glScalef(halfExtent[0]*2, halfExtent[1]*2, halfExtent[2]*2);
	glutWireCube(1);
	glPopMatrix();
}

btCollisionShape* CBoxShape::getBulletShape(){ return Shape; }

///////////////////////////////////////////////////////////////////////////////
// CSphereShape
///////////////////////////////////////////////////////////////////////////////
CSphereShape::CSphereShape(float radius)
{
	Shape=new btSphereShape(radius);
}

CSphereShape::~CSphereShape()
{
	delete Shape;
	Shape=0;
}

void CSphereShape::draw()
{
	glPushMatrix();
	glutWireSphere(Shape->getRadius(), 12, 8);
	glPopMatrix();
}

btCollisionShape* CSphereShape::getBulletShape(){ return Shape; }

///////////////////////////////////////////////////////////////////////////////
// CCapsuleShape
///////////////////////////////////////////////////////////////////////////////
CCapsuleShape::CCapsuleShape(float w, float h)
{
	Shape=new btCapsuleShape(w, h);
}

CCapsuleShape::~CCapsuleShape()
{
	delete Shape;
	Shape=0;
}

void CCapsuleShape::draw()
{
	glPushMatrix();
	glScalef(Shape->getRadius(),
			Shape->getHalfHeight()*2, Shape->getRadius());
	glRotatef(90, 1, 0, 0);
	glutWireSphere(1, 12, 8);
	glPopMatrix();
}

btCollisionShape* CCapsuleShape::getBulletShape(){ return Shape; }

///////////////////////////////////////////////////////////////////////////////
// CPlaneShape
///////////////////////////////////////////////////////////////////////////////
CPlaneShape::CPlaneShape()
{
	Shape=new btStaticPlaneShape(btVector3( 0.0f, 1.0f, 0.0f ), 0.0f);
};

CPlaneShape::~CPlaneShape()
{
	delete Shape;
	Shape=0;
}

void CPlaneShape::draw()
{
}

btCollisionShape* CPlaneShape::getBulletShape(){ return Shape; }

} // namespace
} // namespace

