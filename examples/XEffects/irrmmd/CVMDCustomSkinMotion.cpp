#include "CVMDCustomSkinMotion.h"
#include "SRotPosKey.h"
#include "vmdloader.h"
#include "utility.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace irr {

namespace scene {

using namespace polymesh;

CVMDCustomSkinMotion::CVMDCustomSkinMotion()
: FrameCount(0)
{}

CVMDCustomSkinMotion::~CVMDCustomSkinMotion()
{
	{
		// drop BoneCurveMap
		core::map<core::stringc, SRotPosLinearCurve*>::Iterator it=
			BoneCurveMap.getIterator();
		while(!it.atEnd()){
			it->getValue()->drop();
		}
		BoneCurveMap.clear();
	}

	{
		// drop MorphingCurveMap
		core::map<core::stringc, SMorphLinearCurve*>::Iterator it=
			MorphingCurveMap.getIterator();
		while(!it.atEnd()){
			it->getValue()->drop();
		}
		MorphingCurveMap.clear();
	}
}

bool CVMDCustomSkinMotion::load(const io::path &path)
{
	std::vector<char> buf;
	{
		std::ifstream io(path.c_str(), std::ios::binary);
		if(!io){
			return false;
		}

		// get filesize
		io.seekg(0, std::fstream::end);
		size_t filesize = io.tellg();
		io.clear();
		io.seekg(0, std::fstream::beg);
		if (filesize==0){
			std::cout << "empty file" << std::endl;
			return false;
		}

		// read all file
		buf.resize(filesize);
		io.read(&buf[0], filesize);
	}

	// parse vmd
	vmd::Loader loader;
	if(!loader.parse(&buf[0], buf.size())){
		return false;
	}

	std::cout << loader << std::endl;

	// store bone motions
	u32 lastFrame=0;
	for(vmd::Loader::BoneMap::iterator it=loader.boneMap.begin();
			it!=loader.boneMap.end();
			++it){
		SRotPosLinearCurve *curve=addBoneCurve(it->first.c_str());
		vmd::BoneBuffer::Ptr buf=it->second;
		curve->reallocate(buf->bones.size());
		for(size_t i=0; i<buf->bones.size(); ++i){
			vmd::Bone &frame=buf->bones[i];
			curve->addKeyFrame(frame.frame,
					SRotPosKey(
						core::vector3df(frame.pos.x, frame.pos.y, frame.pos.z),
						core::quaternion(
							frame.q.x, frame.q.y, frame.q.z, frame.q.w))
					);
		}
		curve->sort();
		lastFrame=std::max(lastFrame, curve->getFrameCount());
	}

	// store morphing motions
	for(vmd::Loader::MorphMap::iterator it=loader.morphMap.begin();
			it!=loader.morphMap.end();
			++it){
		SMorphLinearCurve *curve=addMorphingCurve(it->first.c_str());
		vmd::MorphBuffer::Ptr buf=it->second;
		curve->reallocate(buf->morphs.size());
		for(size_t i=0; i<buf->morphs.size(); ++i){
			vmd::Morph &frame=buf->morphs[i];
			curve->addKeyFrame(frame.frame, frame.influence);
		}
		curve->sort();
		lastFrame=std::max(lastFrame, curve->getFrameCount());
	}

	

	FrameCount=lastFrame;

	return true;
}

SRotPosLinearCurve* 
CVMDCustomSkinMotion::getBoneCurve(const core::stringc &name)
{
	core::map<core::stringc, SRotPosLinearCurve*>::Node *found=
		BoneCurveMap.find(name); 
	if(found){
		return found->getValue();
	}
	else{
		return 0;
	}
}

SRotPosLinearCurve* 
CVMDCustomSkinMotion::addBoneCurve(const core::stringc &name)
{
	SRotPosLinearCurve *curve=new SRotPosLinearCurve(name);
	BoneCurveMap.insert(name, curve);
	return curve;
}

SMorphLinearCurve* 
CVMDCustomSkinMotion::getMorphingCurve(const core::stringc &name)
{
	core::map<core::stringc, SMorphLinearCurve*>::Node *found=
		MorphingCurveMap.find(name); 
	if(found){
		return found->getValue();
	}
	else{
		return 0;
	}
}

SMorphLinearCurve* 
CVMDCustomSkinMotion::addMorphingCurve(const core::stringc &name)
{
	SMorphLinearCurve *curve=new SMorphLinearCurve(name);
	MorphingCurveMap.insert(name, curve);
	return curve;
}


} // namespace scene
} // namespace irr
