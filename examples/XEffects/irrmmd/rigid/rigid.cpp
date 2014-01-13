#include "rigid.h"
#include <cassert>

namespace rigid {
  ////////////////////////////////////////////////////////////
  // Rigid
  ////////////////////////////////////////////////////////////
  Rigid 
    Rigid::before(const Rigid &_l)const
    {
      Matrix3 l=quaternionToMatrix(_l.rot_);
      Matrix3 r=quaternionToMatrix(rot_);
      return Rigid(
          quaternionFromMatrix(l * r),
          r.apply(_l.pos_)+pos_
          );
    }

  Rigid 
    Rigid::after(const Rigid &_r)const
    {
      Matrix3 l=quaternionToMatrix(rot_);
      Matrix3 r=quaternionToMatrix(_r.rot_);
      return Rigid(
          quaternionFromMatrix(l * r),
          r.apply(pos_)+_r.pos_          
          );
    }

  Rigid 
    Rigid::getInverse()const
    {
      Matrix3 rot=quaternionToMatrix(rot_);
      rot.transpose();
      return Rigid(quaternionFromMatrix(rot), rot.apply(-pos_));
    }

  Vector3 
    Rigid::apply(const Vector3 &v)const
    {
      return quaternionToMatrix(rot_).apply(v)+pos_;
    }

  Rigid &
    Rigid::normalize()
    {
      rot_.normalize();
      return *this;
    }

  Vector3
    Rigid::getTranslation()const
    {
      return pos_;
    }

  float
    Rigid::getX()const
    {
      return pos_.x;
    }
  float
    Rigid::getY()const
    {
      return pos_.y;
    }
  float
    Rigid::getZ()const
    {
      return pos_.z;
    }

  Matrix3
    Rigid::getRotation()const
    {
      return quaternionToMatrix(rot_);
    }

  /*
  inline Quaternion
    Rigid::getQuaternion()const
    {
      float tr=rot_.trace();
      float w=std::sqrt(tr+1)*0.5;
      if(tr>rot_._00 && tr>rot_._11 && tr>rot_._22){
        float factor=1.0/(w*4.0);
        return Quaternion(
            (rot_._21-rot_._12)*factor,
            (rot_._02-rot_._20)*factor,
            (rot_._10-rot_._01)*factor,
            w
            );
      }
      else{
        return Quaternion(
            std::sqrt( rot_._00-rot_._11-rot_._22+1)*0.5,
            std::sqrt(-rot_._00+rot_._11-rot_._22+1)*0.5,
            std::sqrt(-rot_._00-rot_._11+rot_._22+1)*0.5,
            std::sqrt(tr+1)*0.5);
      }
    }
  */
  float SIGN(float x) {return (x >= 0.0f) ? +1.0f : -1.0f;}

  Quaternion 
    Rigid::getQuaternion()const
    {
      return rot_;
    }

  Matrix4 
    Rigid::getMatrix()const
    {
      Matrix3 m=quaternionToMatrix(rot_);
      return Matrix4(
          m._00, m._01, m._02, 0,
          m._10, m._11, m._12, 0,
          m._20, m._21, m._22, 0,
          pos_.x, pos_.y, pos_.z, 1.0
          );
    }

  ////////////////////////////////////////////////////////////
  // functions
  ////////////////////////////////////////////////////////////
  Matrix3
    quaternionToMatrix(const Quaternion &rot)
    {
      float xx=rot.x*rot.x;
      float yy=rot.y*rot.y;
      float zz=rot.z*rot.z;

      float xy=rot.x*rot.y;
      float yz=rot.y*rot.z;
      float zx=rot.z*rot.x;

      float wx=rot.w*rot.x;
      float wy=rot.w*rot.y;
      float wz=rot.w*rot.z;

      /*
         Matrix3 m(
         1-2*(yy+zz), 2*(xy-wz), 2*(zx+wy),
         2*(xy+wz), 1-2*(xx+zz), 2*(yz-wx),
         2*(zx-wy), 2*(yz+wx), 1-2*(xx+yy)
         );
         */
      // transpose
      Matrix3 m(
          1-2*(yy+zz), 2*(xy+wz), 2*(zx-wy),
          2*(xy-wz), 1-2*(xx+zz), 2*(yz+wx),
          2*(zx+wy), 2*(yz-wx), 1-2*(xx+yy)
          );
      return m;
    }

  Quaternion
    quaternionAxisRadian(const Vector3 &axis, float radian)
    {
      float half_rad=static_cast<float>(radian/2.0);
      float c=static_cast<float>(std::cos(half_rad));
      float s=static_cast<float>(std::sin(half_rad));
      return Quaternion(axis.x*s, axis.y*s, axis.z*s, c);
    }

  Vector3 quaternionToEulerRadian(const Quaternion &q)
  {
    float	x2 = q.x + q.x;
    float	y2 = q.y + q.y;
    float	z2 = q.z + q.z;
    float	xz2 = q.x * z2;
    float	wy2 = q.w * y2;
    float	temp = -(xz2 - wy2);

    if( temp >= 1.f ){ 
      temp = 1.f; 
    }
    else if( temp <= -1.f ){ 
      temp = -1.f; 
    }

    float	yRadian = asinf(temp);

    float	xx2 = q.x * x2;
    float	xy2 = q.x * y2;
    float	zz2 = q.z * z2;
    float	wz2 = q.w * z2;

    Vector3 angle;
    if( yRadian < M_PI * 0.5f )
    {
      if( yRadian > -M_PI * 0.5f )
      {
        float	yz2 = q.y * z2;
        float	wx2 = q.w * x2;
        float	yy2 = q.y * y2;
        angle.x = atan2f( (yz2 + wx2), (1.f - (xx2 + yy2)) );
        angle.y = yRadian;
        angle.z = atan2f( (xy2 + wz2), (1.f - (yy2 + zz2)) );
      }
      else
      {
        angle.x = -atan2f( (xy2 - wz2), (1.f - (xx2 + zz2)) );
        angle.y = yRadian;
        angle.z = 0.f;
      }
    }
    else
    {
      angle.x = atan2f( (xy2 - wz2), (1.f - (xx2 + zz2)) );
      angle.y = yRadian;
      angle.z = 0.f;
    }

    return angle;
  }

  Rigid
    translation(float x, float y, float z)
    {
      return Rigid(Vector3(x, y, z));
    }

  Rigid
    translation(const Vector3 &v)
    {
      return Rigid(v);
    }

  Rigid
    rotationAndTranslation(const Quaternion &rot, const Vector3 &pos)
    {
      return Rigid(rot, pos);
    }

  Rigid
    rotationAndTranslation(const Matrix3 &m, const Vector3 &pos)
    {
      return Rigid(quaternionFromMatrix(m), pos);
    }

  Rigid 
    rotationX(float rad)
    {
      // ToDo
      return Rigid();
    }

  Rigid 
    rotationY(float rad)
    {
      // ToDo
      return Rigid();
    }

  Rigid 
    rotationZ(float rad)
    {
      // ToDo
      return Rigid();
    }

  Rigid 
    rotation(const Quaternion &q)
    {
      return Rigid(q);
    }

  Rigid
    rotation(const Matrix3 &m)
    {
      return Rigid(quaternionFromMatrix(m));
    }

  Rigid
    affine(const Matrix4 &m)
    {
      // no check?
      return rotationAndTranslation(
          quaternionFromMatrix(Matrix3(
              m._00, m._01, m._02,
              m._10, m._11, m._12,
              m._20, m._21, m._22
              )), 
          Vector3(m._30, m._31, m._32)
          );
    }

  Rigid 
    rotationAxisRadian(const Vector3 &axis, float rad)
    {
      return Rigid(quaternionAxisRadian(axis, rad));
    }

  Rigid 
    rotationPitchHeadBankRadian(float pitch, float yaw, float roll)
    {
      float xRadian = pitch * 0.5f;
      float yRadian = yaw * 0.5f;
      float zRadian = roll * 0.5f;
      float sinX = sinf( xRadian );
      float cosX = cosf( xRadian );
      float sinY = sinf( yRadian );
      float cosY = cosf( yRadian );
      float sinZ = sinf( zRadian );
      float cosZ = cosf( zRadian );

      return rotation(Quaternion(
            sinX * cosY * cosZ - cosX * sinY * sinZ,
            cosX * sinY * cosZ + sinX * cosY * sinZ,
            cosX * cosY * sinZ - sinX * sinY * cosZ,
            cosX * cosY * cosZ + sinX * sinY * sinZ
            ));
    }
  Rigid 
    rotationPitchHeadBankRadian(const Vector3 &euler)
    {
      return rotationPitchHeadBankRadian(euler.x, euler.y, euler.z);
    }

  Quaternion
    quaternionFromMatrix(const Matrix3 &m)
    {
      double fourWSquaredMinus1=m._00+m._11+m._22;
      double fourXSquaredMinus1=m._00-m._11-m._22;
      double fourYSquaredMinus1=m._11-m._00-m._22;
      double fourZSquaredMinus1=m._22-m._00-m._11;

      int biggestIndex=0;
      double fourBiggestSquredMinus1=fourWSquaredMinus1;
      if(fourXSquaredMinus1>fourBiggestSquredMinus1){
        fourBiggestSquredMinus1=fourXSquaredMinus1;
        biggestIndex=1;
      }
      if(fourYSquaredMinus1>fourBiggestSquredMinus1){
        fourBiggestSquredMinus1=fourYSquaredMinus1;
        biggestIndex=2;
      }
      if(fourZSquaredMinus1>fourBiggestSquredMinus1){
        fourBiggestSquredMinus1=fourZSquaredMinus1;
        biggestIndex=3;
      }

      double biggestVal=std::sqrt(fourBiggestSquredMinus1+1.0f)*0.5f;
      double mult=0.25f/biggestVal;

      switch(biggestIndex)
      {
      case 0:
        return Quaternion(
            static_cast<float>((m._12-m._21)*mult),
            static_cast<float>((m._20-m._02)*mult),
            static_cast<float>((m._01-m._10)*mult),
            static_cast<float>(biggestVal)
            );
      case 1:
        return Quaternion(
            static_cast<float>(biggestVal),
            static_cast<float>((m._01+m._10)*mult),
            static_cast<float>((m._20+m._02)*mult),
            static_cast<float>((m._12-m._21)*mult)
            );
      case 2:
        return Quaternion(
            static_cast<float>((m._01+m._10)*mult),
            static_cast<float>(biggestVal),
            static_cast<float>((m._12+m._21)*mult),
            static_cast<float>((m._20-m._02)*mult)
            );
      case 3:
        return Quaternion(
            static_cast<float>((m._20+m._02)*mult),
            static_cast<float>((m._12+m._21)*mult),
            static_cast<float>(biggestVal),
            static_cast<float>((m._01-m._10)*mult)
            );
      default:
        assert(false);
        return Quaternion();
      }
    }

  Quaternion 
    quaternionFromAxisRadian(const Vector3 &axis, float radian)
    {
      // ToDo
      return Quaternion();
    }

}
