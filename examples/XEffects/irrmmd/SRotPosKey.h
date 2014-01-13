#ifndef ROT_POS_KEY_H_INCLUDED
#define ROT_POS_KEY_H_INCLUDED

#include "CCurve.h"

namespace irr {
namespace scene {

struct SRotPosKey
{
	core::vector3df Position;
	core::quaternion Rotation;

	SRotPosKey()
		: 
		Position(core::vector3df(0, 0, 0)), 
		Rotation( core::quaternion(0, 0, 0, 1))
		{}

	SRotPosKey(
			const core::vector3df &pos,
			const core::quaternion &rot)
		: Position(pos), Rotation(rot)
		{}

};
inline SRotPosKey lerp(
		const SRotPosKey &start, const SRotPosKey &end, f32 factor)
{
	return SRotPosKey(
			//start.Position.getInterpolated(end.Position, factor),
			core::vector3df(
				start.Position.X+(end.Position.X-start.Position.X)*factor,
				start.Position.Y+(end.Position.Y-start.Position.Y)*factor,
				start.Position.Z+(end.Position.Z-start.Position.Z)*factor
				),
			core::quaternion().slerp(
				start.Rotation, end.Rotation, factor)
			);
}

typedef CLinearCurve<SRotPosKey> SRotPosLinearCurve;

} // namespace scene
} // namespace irr

#endif // ROT_POS_KEY_H_INCLUDED
