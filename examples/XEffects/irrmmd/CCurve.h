#ifndef CURVE_H_INCLUDED
#define CURVE_H_INCLUDED

#include <irrlicht.h>
#include <assert.h>
#include <algorithm>

namespace irr {
namespace scene {

template<typename SKey>
class CLinearCurve : public IReferenceCounted
{
public:
	struct SKeyFrame
	{
		u16 Frame;
		SKey Key;

		SKeyFrame(u16 frame, const SKey &key)
			: Frame(frame), Key(key)
			{}

		bool operator<(const SKeyFrame &rhs)const
		{
			return Frame<rhs.Frame;
		}
	};

	CLinearCurve(const core::stringc &name)
		: Name(name)
	{}

	SKey getKey(s32 frame)
	{
		// todo: binary search
		if(KeyFrames.empty()){
			return SKey();
		}
		if(frame<KeyFrames[0].Frame){
			return KeyFrames[0].Key;
		}
		size_t i=0;
		for(; i<KeyFrames.size(); ++i){
			if(KeyFrames[i].Frame==frame){
				return KeyFrames[i].Key;
			}
			if(KeyFrames[i].Frame>=frame){
				break;
			}
		}
		if(i==KeyFrames.size()){
			return KeyFrames.getLast().Key;
		}
		assert(i>0);
		SKeyFrame start(KeyFrames[i-1]);
		SKeyFrame end(KeyFrames[i]);

		f32 factor=static_cast<f32>(frame-start.Frame)/(end.Frame-start.Frame);

		return lerp(start.Key, end.Key, factor);
	}

	void addKeyFrame(u16 frame, const SKey &key)
	{
		KeyFrames.push_back(SKeyFrame(frame, key));
	}

	void reallocate(u16 size){ KeyFrames.reallocate(size); }

	void sort(){
		KeyFrames.sort();
	}

	u32 getFrameCount()const
	{
		if(KeyFrames.empty()){
			return 0;
		}
		return KeyFrames.getLast().Frame+1;
	}

private:
	core::stringc Name;
	core::array<SKeyFrame> KeyFrames;
};

} // namespace irr
} // namespace scene

#endif // CURVE_H_INCLUDED
