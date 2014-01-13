#ifndef RIGID_MATRIX_H

#include "vec.h"

namespace rigid {

  struct Matrix3
  {
    float _00, _01, _02;
    float _10, _11, _12;
    float _20, _21, _22;

    Matrix3()
    {
      _00=1.0f; _01=0.0f; _02=0.0f;
      _10=0.0f; _11=1.0f; _12=0.0f;
      _20=0.0f; _21=0.0f; _22=1.0f;
    }

    Matrix3(
        float m00, float m01, float m02,
        float m10, float m11, float m12, 
        float m20, float m21, float m22
        )
    {
      _00=m00; _01=m01; _02=m02;
      _10=m10; _11=m11; _12=m12;
      _20=m20; _21=m21; _22=m22;
    }

    float trace()const
    {
      return _00+_11+_22;
    }

    Matrix3 &transpose()
    {
      std::swap(_01, _10);
      std::swap(_02, _20);
      std::swap(_12, _21);
      return *this;
    }

    Matrix3 operator*(float rhs)const
    {
      Matrix3 m=*this;
      m._00*=rhs; m._01*=rhs; m._02*=rhs;
      m._10*=rhs; m._11*=rhs; m._12*=rhs;
      m._20*=rhs; m._21*=rhs; m._22*=rhs;
      return m;
    }

    Matrix3 operator*(const Matrix3 &rhs)const
    {
      Matrix3 m;
      m._00= _00*rhs._00 + _01*rhs._10 + _02*rhs._20;
      m._01= _00*rhs._01 + _01*rhs._11 + _02*rhs._21;
      m._02= _00*rhs._02 + _01*rhs._12 + _02*rhs._22;

      m._10= _10*rhs._00 + _11*rhs._10 + _12*rhs._20;
      m._11= _10*rhs._01 + _11*rhs._11 + _12*rhs._21;
      m._12= _10*rhs._02 + _11*rhs._12 + _12*rhs._22;

      m._20= _20*rhs._00 + _21*rhs._10 + _22*rhs._20;
      m._21= _20*rhs._01 + _21*rhs._11 + _22*rhs._21;
      m._22= _20*rhs._02 + _21*rhs._12 + _22*rhs._22;

      return m;
    }

    Matrix3 operator+(const Matrix3 &rhs)const
    {
      Matrix3 m;

      m._00=_00+rhs._00; m._01=_01+rhs._01; m._02=_02+rhs._02;
      m._10=_10+rhs._10; m._11=_11+rhs._11; m._12=_12+rhs._12;
      m._20=_20+rhs._20; m._21=_21+rhs._21; m._22=_22+rhs._22;

      return m;
    }

    Matrix3 operator-(const Matrix3 &rhs)const
    {
      Matrix3 m;

      m._00=_00-rhs._00; m._01=_01-rhs._01; m._02=_02-rhs._02;
      m._10=_10-rhs._10; m._11=_11-rhs._11; m._12=_12-rhs._12;
      m._20=_20-rhs._20; m._21=_21-rhs._21; m._22=_22-rhs._22;

      return m;
    }

    Vector3 apply(const Vector3 &v)const
    {
      return Vector3(
          v.x*_00 + v.y*_10 + v.z*_20,
          v.x*_01 + v.y*_11 + v.z*_21,
          v.x*_02 + v.y*_12 + v.z*_22
          );
    }

    static Matrix3 lerp(const Matrix3 &start, const Matrix3 &end,
        float factor)
    {
        return Matrix3(
            start._00+(end._00-start._00)*factor,
            start._01+(end._01-start._01)*factor,
            start._02+(end._02-start._02)*factor,
            start._10+(end._10-start._10)*factor,
            start._11+(end._11-start._11)*factor,
            start._12+(end._12-start._12)*factor,
            start._20+(end._20-start._20)*factor,
            start._21+(end._21-start._21)*factor,
            start._22+(end._22-start._22)*factor);
    }
  };

  inline std::ostream& operator<<(std::ostream &os, const Matrix3 &rhs)
  {
    os
      << '[' << rhs._00 << ',' << rhs._01 << ',' << rhs._02 <<  ']' << std::endl
      << '[' << rhs._10 << ',' << rhs._11 << ',' << rhs._12 <<  ']' << std::endl
      << '[' << rhs._20 << ',' << rhs._21 << ',' << rhs._22 <<  ']' << std::endl
      ;
    return os;
  }

  struct Matrix4
  {
    float _00, _01, _02, _03;
    float _10, _11, _12, _13;
    float _20, _21, _22, _23;
    float _30, _31, _32, _33;

    Matrix4()
    {
      _00=1.0f; _01=0.0f; _02=0.0f; _03=0.0f;
      _10=0.0f; _11=1.0f; _12=0.0f; _13=0.0f;
      _20=0.0f; _21=0.0f; _22=1.0f; _23=0.0f;
      _30=0.0f; _31=0.0f; _32=0.0f; _33=1.0f;
    }

    Matrix4(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13, 
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33
        )
    {
      _00=m00; _01=m01; _02=m02; _03=m03;
      _10=m10; _11=m11; _12=m12; _13=m13;
      _20=m20; _21=m21; _22=m22; _23=m23;
      _30=m30; _31=m31; _32=m32; _33=m33;
    }

    Matrix4 &transpose()
    {
      std::swap(_01, _10);
      std::swap(_02, _20);
      std::swap(_03, _30);
      std::swap(_12, _21);
      std::swap(_13, _31);
      std::swap(_23, _32);
      return *this;
    }

    Matrix4 operator*(float rhs)const
    {
      Matrix4 m=*this;
      m._00*=rhs; m._01*=rhs; m._02*=rhs; m._03*=rhs;
      m._10*=rhs; m._11*=rhs; m._12*=rhs; m._13*=rhs;
      m._20*=rhs; m._21*=rhs; m._22*=rhs; m._23*=rhs;
      m._30*=rhs; m._31*=rhs; m._32*=rhs; m._33*=rhs;
      return m;
    }

    Matrix4 operator*(const Matrix4 &rhs)const
    {
      Matrix4 m;
      m._00=_00*rhs._00 + _01*rhs._10 + _02*rhs._20 + _03*rhs._30;
      m._01=_00*rhs._01 + _01*rhs._11 + _02*rhs._21 + _03*rhs._31;
      m._02=_00*rhs._02 + _01*rhs._12 + _02*rhs._22 + _03*rhs._32;
      m._03=_00*rhs._03 + _01*rhs._13 + _02*rhs._23 + _03*rhs._33;

      m._10=_10*rhs._00 + _11*rhs._10 + _12*rhs._20 + _13*rhs._30;
      m._11=_10*rhs._01 + _11*rhs._11 + _12*rhs._21 + _13*rhs._31;
      m._12=_10*rhs._02 + _11*rhs._12 + _12*rhs._22 + _13*rhs._32;
      m._13=_10*rhs._03 + _11*rhs._13 + _12*rhs._23 + _13*rhs._33;

      m._20=_20*rhs._00 + _21*rhs._10 + _22*rhs._20 + _23*rhs._30;
      m._21=_20*rhs._01 + _21*rhs._11 + _22*rhs._21 + _23*rhs._31;
      m._22=_20*rhs._02 + _21*rhs._12 + _22*rhs._22 + _23*rhs._32;
      m._23=_20*rhs._03 + _21*rhs._13 + _22*rhs._23 + _23*rhs._33;

      m._30=_30*rhs._00 + _31*rhs._10 + _32*rhs._20 + _33*rhs._30;
      m._31=_30*rhs._01 + _31*rhs._11 + _32*rhs._21 + _33*rhs._31;
      m._32=_30*rhs._02 + _31*rhs._12 + _32*rhs._22 + _33*rhs._32;
      m._33=_30*rhs._03 + _31*rhs._13 + _32*rhs._23 + _33*rhs._33;
      return m;
    }

    Matrix4 operator+(const Matrix4 &rhs)const
    {
      Matrix4 m;

      m._00=_00+rhs._00; m._01=_01+rhs._01; m._02=_02+rhs._02; m._03=_03+rhs._03;
      m._10=_10+rhs._10; m._11=_11+rhs._11; m._12=_12+rhs._12; m._13=_13+rhs._13;
      m._20=_20+rhs._20; m._21=_21+rhs._21; m._22=_22+rhs._22; m._23=_23+rhs._23;
      m._30=_30+rhs._30; m._31=_31+rhs._31; m._32=_32+rhs._32; m._33=_33+rhs._33;

      return m;
    }

    Matrix4 operator-(const Matrix4 &rhs)const
    {
      Matrix4 m;

      m._00=_00-rhs._00; m._01=_01-rhs._01; m._02=_02-rhs._02; m._03=_03-rhs._03;
      m._10=_10-rhs._10; m._11=_11-rhs._11; m._12=_12-rhs._12; m._13=_13-rhs._13;
      m._20=_20-rhs._20; m._21=_21-rhs._21; m._22=_22-rhs._22; m._23=_23-rhs._23;
      m._30=_30-rhs._30; m._31=_31-rhs._31; m._32=_32-rhs._32; m._33=_33-rhs._33;

      return m;
    }

    static Matrix4 lerp(const Matrix4 &start, const Matrix4 &end,
        float factor)
    {
        return Matrix4(
            start._00+(end._00-start._00)*factor,
            start._01+(end._01-start._01)*factor,
            start._02+(end._02-start._02)*factor,
            start._03+(end._03-start._03)*factor,
            start._10+(end._10-start._10)*factor,
            start._11+(end._11-start._11)*factor,
            start._12+(end._12-start._12)*factor,
            start._13+(end._13-start._13)*factor,
            start._20+(end._20-start._20)*factor,
            start._21+(end._21-start._21)*factor,
            start._22+(end._22-start._22)*factor,
            start._23+(end._23-start._23)*factor,
            start._30+(end._30-start._30)*factor,
            start._31+(end._31-start._31)*factor,
            start._32+(end._32-start._32)*factor,
            start._33+(end._33-start._33)*factor);
    }
  };

  inline std::ostream& operator<<(std::ostream &os, const Matrix4 &rhs)
  {
    os
      << '[' << rhs._00 << ',' << rhs._01 << ',' << rhs._02 << ',' << rhs._03 << ']' << std::endl
      << '[' << rhs._10 << ',' << rhs._11 << ',' << rhs._12 << ',' << rhs._13 << ']' << std::endl
      << '[' << rhs._20 << ',' << rhs._21 << ',' << rhs._22 << ',' << rhs._23 << ']' << std::endl
      << '[' << rhs._30 << ',' << rhs._31 << ',' << rhs._32 << ',' << rhs._33 << ']' << std::endl
      ;
    return os;
  }

}

#endif // RIGID_MATRIX_H
