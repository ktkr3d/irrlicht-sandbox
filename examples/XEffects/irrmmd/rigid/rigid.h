#ifndef RIGID_H
#define RIGID_H

#include "matrix.h"
#include "quaternion.h"

namespace rigid {

  enum RIGHTUPAWAY_AXIS
  {
    // RIGHTUPAWAY_{Positive|Negative}{X|Y|Z}
    RIGHTUPAWAY_PXPYPZ, // DirectX. left.handed y-up
    RIGHTUPAWAY_PXPYNZ, // OpenGL. right-handed y-up
    RIGHTUPAWAY_UNKNOWN,
  };

  class Rigid
  {
    //Matrix3 rot_;
    Quaternion rot_;
    Vector3 pos_;

  public:
    Rigid()
    {}
    Rigid(const Quaternion &rot)
      : rot_(rot)
      {}
    Rigid(const Quaternion &rot, const Vector3 &pos)
      : rot_(rot), pos_(pos)
      {}
    Rigid(const Vector3 &pos)
      : pos_(pos)
      {}
    Rigid before(const Rigid &r)const;
    Rigid after(const Rigid &r)const;
    Rigid getInverse()const;
    Vector3 apply(const Vector3 &v)const;

    Rigid &normalize();

    Vector3 getTranslation()const;
    float getX()const;
    float getY()const;
    float getZ()const;
    Matrix3 getRotation()const;
    Quaternion getQuaternion()const;
    Matrix4 getMatrix()const;

    static Rigid
      lerp(const Rigid &start, const Rigid &end, float factor);
  };
  inline std::ostream &operator<<(std::ostream &os, const Rigid rhs)
  {
    os 
      << "<Rigid "
      << rhs.getRotation()
      << rhs.getTranslation()
      << ">"
      ;
    return os;
  }

  Quaternion quaternionFromMatrix(const Matrix3 &m);
  Quaternion quaternionFromAxisRadian(const Vector3 &axis, float radian);
  std::pair<Vector3, float> axisAngleFromQuaternion(const Quaternion &q);
  Matrix3 quaternionToMatrix(const Quaternion &q);
  Vector3 quaternionToEulerRadian(const Quaternion &q);
  Rigid translation(float x, float y, float z);
  Rigid translation(const Vector3 &pos);
  Rigid rotationAndTranslation(const Quaternion &rot, const Vector3 &pos);
  Rigid rotationAndTranslation(const Matrix3 &m, const Vector3 &pos);
  Rigid rotationX(float rad);
  Rigid rotationY(float rad);
  Rigid rotationZ(float rad);
  Rigid rotation(const Quaternion &q);
  Rigid rotation(const Matrix3 &m);
  Rigid rotationAxisRadian(const Vector3 &axis, float rad);
  Rigid rotationPitchHeadBankRadian(float pitch, float head, float bank);
  Rigid rotationPitchHeadBankRadian(const Vector3 &euler);
  Rigid affine(const Matrix4 &m);

  struct MatrixArray44
  {
    float _[16];

    void assign(const Rigid &r)
    {
      Matrix3 m=r.getRotation();
      Vector3 v=r.getTranslation();
      /*
      _[0]=m._00; _[4]=m._01; _[8]=m._02; _[12]=0;
      _[1]=m._10; _[5]=m._11; _[9]=m._12; _[13]=0;
      _[2]=m._20; _[6]=m._21; _[10]=m._22; _[14]=0;
      _[3]=v.x; _[7]=v.y; _[11]=v.z; _[15]=1;
      */
      _[0]=m._00; _[1]=m._01; _[2]=m._02; _[3]=0;
      _[4]=m._10; _[5]=m._11; _[6]=m._12; _[7]=0;
      _[8]=m._20; _[9]=m._21; _[10]=m._22; _[11]=0;
      _[12]=v.x; _[13]=v.y; _[14]=v.z; _[15]=1;
    }

    float *get(){ return &_[0]; }
  };
}
#endif // RIGID_H
