#ifndef RIGID_QUATERNION_H
#define RIGID_QUATERNION_H

#ifndef _MSC_VER
//#include <windows.h>
#endif
#include <cmath>
#include <ostream>

namespace rigid {

  struct Quaternion
  {
    float x;
    float y;
    float z;
    float w;

    Quaternion(float _x=0, float _y=0, float _z=0, float _w=1)
      : x(_x), y(_y), z(_z), w(_w)
      {}

    Quaternion operator*(const Quaternion &rhs);

    double sqNorm()const
    {
      return x*x + y*y + z*z + w*w;
    }

    double norm()const
    {
      return std::sqrt(sqNorm());
    }

    Quaternion &normalize()
    {
      float factor=static_cast<float>(1.0/norm());
      x*=factor;
      y*=factor;
      z*=factor;
      w*=factor;
      return *this;
    }

    static Quaternion slerp(const Quaternion &start, const Quaternion &end,
        float t);

    static double dot(const Quaternion &lhs, const Quaternion &rhs)
    {
      return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z + lhs.w*rhs.w;
    }
  };
  inline std::ostream &operator<<(std::ostream &os, const Quaternion &rhs)
  {
    return os
      << '[' << rhs.x << ',' <<rhs.y << ',' << rhs.z << ',' << rhs.w << ']';
  }

}

#endif // RIGID_QUATERNION_H
