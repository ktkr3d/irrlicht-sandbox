#ifndef SCURVE_H_INCLUDED
#define SCURVE_H_INCLUDED

namespace irr {
namespace scene {

template<typename SKey> class CLinearCurve;
struct SRotPosKey;
typedef CLinearCurve<SRotPosKey> SRotPosLinearCurve;

typedef CLinearCurve<f32> SMorphLinearCurve;
inline f32 lerp(f32 start, f32 end, f32 factor)
{
	return start+(end-start)*factor;
}

} // namespace irr
} // namespace scene

#endif // SCURVE_H_INCLUDED
