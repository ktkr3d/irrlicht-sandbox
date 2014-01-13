#include "quaternion.h"
#include <cassert>

namespace rigid {
  Quaternion
    Quaternion::slerp(const Quaternion &start, const Quaternion &end, float t)
    {
      /*
         if(t<=0.0f)
         return start;
         if(t>=1.0f)
         return end;

         double cosOmega=Quaternion::dot(start, end);

         double q1w=end.w;
         double q1x=end.x;
         double q1y=end.y;
         double q1z=end.z;
         if(cosOmega<0.0){
         q1w=-q1w;
         q1x=-q1x;
         q1y=-q1y;
         q1z=-q1z;
         cosOmega=-cosOmega;
         }
         assert(cosOmega<1.1);

         double k0;
         double k1;
         if(cosOmega>0.9999){
         k0=1.0-t;
         k1=t;
         }
         else{
         double sinOmeta=std::sqrt(1.0-cosOmega*cosOmega);
         double omega=std::atan2(sinOmeta, cosOmega);
         double oneOverSinOmega=1.0/sinOmeta;
         k0=std::sin((1.0-t)*omega)*oneOverSinOmega;
         k1=std::sin(t*omega)*oneOverSinOmega;
         }

         return Quaternion(
         static_cast<float>(k0*start.x + k1*q1x),
         static_cast<float>(k0*start.y + k1*q1y),
         static_cast<float>(k0*start.z + k1*q1z),
         static_cast<float>(k0*start.w + k1*q1w)
         );
         */
      float qr = 
        start.x * end.x + 
        start.y * end.y + 
        start.z * end.z + 
        start.w * end.w;
      float t0 = 1.0f - t;

      if( qr < 0 ) {
        return Quaternion(
            start.x * t0 - end.x * t,
            start.y * t0 - end.y * t,
            start.z * t0 - end.z * t,
            start.w * t0 - end.w * t
            ).normalize();
      }
      else {
        return Quaternion(
            start.x * t0 + end.x * t,
            start.y * t0 + end.y * t,
            start.z * t0 + end.z * t,
            start.w * t0 + end.w * t
            ).normalize();
      }
    }
}
