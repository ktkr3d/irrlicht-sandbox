#ifndef RIGID_VEC_H
/* required. define in ide or make
#define _USE_MATH_DEFINES
*/
#include <cmath>
#include <sstream>

namespace rigid {

  inline float lerp(float s, float e, float t)
  {
    return s+(e-s)*t;
  }

  ////////////////////////////////////////////////////////////
  // Vector2
  ////////////////////////////////////////////////////////////
  struct Vector2
  {
    float x;
    float y;

    Vector2(float _x=0, float _y=0)
      : x(_x), y(_y)
      {}
  };
  inline std::ostream &operator<<(std::ostream &os, const Vector2 &rhs)
  {
    return os
      << '[' << rhs.x << ',' <<rhs.y << ']';
  }

  ////////////////////////////////////////////////////////////
  // Vector3
  ////////////////////////////////////////////////////////////
  struct Vector3
  {
    float x;
    float y;
    float z;

    Vector3(float _x=0, float _y=0, float _z=0)
      : x(_x), y(_y), z(_z)
      {}

    Vector3 operator+(const Vector3 &rhs)const
    {
      return Vector3( x+rhs.x, y+rhs.y, z+rhs.z);
    }
    Vector3 &operator+=(const Vector3 &rhs)
    {
      x+=rhs.x;
      y+=rhs.y;
      z+=rhs.z;
      return *this;
    }

    Vector3 operator-(const Vector3 &rhs)const
    {
      return Vector3(x-rhs.x, y-rhs.y, z-rhs.z);
    }

    Vector3 operator-()const
    {
      return Vector3(-x, -y, -z);
    }

    Vector3 operator*(float rhs)const
    {
      return Vector3(x*rhs, y*rhs, z*rhs);
    }

    bool operator==(const Vector3 &rhs)const
    {
      return x==rhs.x && y==rhs.y && z==rhs.z;
    }

    double sqNorm()const
    {
      return dot(*this, *this);
    }

    double norm()const
    {
      return std::sqrt(sqNorm());
    }

    void normalize()
    {
      float factor=static_cast<float>(1.0/norm());
      x*=factor;
      y*=factor;
      z*=factor;
    }

    static double dot(const Vector3 &lhs, const Vector3 &rhs)
    {
      return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
    }

    static Vector3 cross(const Vector3 &lhs, const Vector3 &rhs)
    {
      return Vector3(
          lhs.y*rhs.z - lhs.z*rhs.y,
          lhs.z*rhs.x - lhs.x*rhs.z,
          lhs.x*rhs.y - lhs.y*rhs.x
          );
    }

    static Vector3 lerp(const Vector3 &start, const Vector3 &end,
        float factor)
    {
      return Vector3(
          ::rigid::lerp(start.x, end.x, factor),
          ::rigid::lerp(start.y, end.y, factor),
          ::rigid::lerp(start.z, end.z, factor)
          );
    }

    std::string to_string()const
    {
      std::stringstream ss;
      ss
        << '[' << x << ',' <<y << ',' << z << ']';
      return ss.str();
    }
  };
  inline std::ostream &operator<<(std::ostream &os, const Vector3 &rhs)
  {
    return os << rhs.to_string();
  }

  ////////////////////////////////////////////////////////////
  // Vector4
  ////////////////////////////////////////////////////////////
  struct Vector4
  {
    float x;
    float y;
    float z;
    float w;

    Vector4()
      : x(0), y(0), z(0), w(0)
      {}

    Vector4(float _x, float _y, float _z, float _w)
      : x(_x), y(_y), z(_z), w(_w)
      {}

    Vector4(const Vector3 &src, float _w)
      : x(src.x), y(src.y), z(src.z), w(_w)
      {}
  };
  inline std::ostream &operator<<(std::ostream &os, const Vector4 &rhs)
  {
    return os
      << '[' << rhs.x << ',' <<rhs.y << ',' << rhs.z << ',' << rhs.w << ']';
  }

}

#endif // RIGID_VEC_H
