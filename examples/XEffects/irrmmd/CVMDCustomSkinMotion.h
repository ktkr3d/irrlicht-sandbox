#ifndef CLASS_VMD_CUSTOM_SKIN_MESH_H_INCLUDED
#define CLASS_VMD_CUSTOM_SKIN_MESH_H_INCLUDED

#include <irrlicht.h>
#include "SCurve.h"

namespace irr {
namespace scene {

class CVMDCustomSkinMotion : public IReferenceCounted
{
public:
	CVMDCustomSkinMotion();
	~CVMDCustomSkinMotion();

	u32 getFrameCount()const{ return FrameCount; }
	bool load(const io::path &path);

	SRotPosLinearCurve* getBoneCurve(const core::stringc &name);
	SRotPosLinearCurve* addBoneCurve(const core::stringc &name);

	SMorphLinearCurve* getMorphingCurve(const core::stringc &name);
	SMorphLinearCurve* addMorphingCurve(const core::stringc &name);

private:
	u32 FrameCount;
	core::map<core::stringc, SRotPosLinearCurve*> BoneCurveMap;
	core::map<core::stringc, SMorphLinearCurve*> MorphingCurveMap;
};

} // namespace scene
} // namespace irr

#endif // CLASS_CUSTOM_SKIN_MESH_H_INCLUDED
