#ifndef CUSTOM_SKINNED_MESH_H_INCLUDED
#define CUSTOM_SKINNED_MESH_H_INCLUDED

#include <irrlicht.h>
#include "CVMDCustomSkinMotion.h"
#include "SCurve.h"

namespace irr {
namespace bullet {

class CWorld;

}
namespace scene {

class IJoint;

//! two weighted skinning class.
class CCustomSkinnedMesh : public IAnimatedMesh
{
public:
	CCustomSkinnedMesh();
	virtual ~CCustomSkinnedMesh();

	////////////////////////////////////////////////////////////
	// IMesh
	////////////////////////////////////////////////////////////
	virtual u32 getMeshBufferCount(void) const;
	virtual IMeshBuffer *getMeshBuffer(const video::SMaterial &) const;
	virtual IMeshBuffer *getMeshBuffer(u32) const;
	virtual const core::aabbox3df &getBoundingBox(void) const;
	virtual void setBoundingBox(const core::aabbox3df &);
	virtual void setMaterialFlag(video::E_MATERIAL_FLAG,bool);
	virtual void setHardwareMappingHint(E_HARDWARE_MAPPING,E_BUFFER_TYPE);
	virtual void setDirty(E_BUFFER_TYPE);

	////////////////////////////////////////////////////////////
	// IAnimatedMesh
	////////////////////////////////////////////////////////////
	virtual u32 getFrameCount(void) const;
	virtual f32 getAnimationSpeed() const;
	virtual void setAnimationSpeed(f32);
	virtual IMesh *getMesh(s32,s32,s32,s32);

	//! Returns the IMesh interface for a frame.
	//! owner and jointChildSceneNodes is for skinning.
	//! The purpose of other arguments is backward compatibility.

	////////////////////////////////////////////////////////////
	// own interface
	////////////////////////////////////////////////////////////
	void recalculateBoundingBox();
	SSharedMeshBuffer* addIndexBuffer();
	void reallocate(u16 size);
	void addVertex(const video::S3DVertex &vertex,
			f32 weight, u16 bone0, u16 bone1);
	void setVertexColor(u16 index, const video::SColor &color);
	void updateBone(u32 frame);
	void deform(u32 frame);
	void setMotion(CVMDCustomSkinMotion *motion);
	void setFrame(u32 frame);

	////////////////////////////////////////////////////////////
	// joints
	////////////////////////////////////////////////////////////
	IJoint* addJoint(const core::stringc &name, const core::vector3df &pos);
	u16 getJointCount()const{ return Joints.size(); }
	IJoint* getRootJoint()const{ return Root; }
	IJoint* getJoint(u16 index){ return Joints[index]; }

	////////////////////////////////////////////////////////////
	// ik resolver
	////////////////////////////////////////////////////////////
	class CIkResolver : public IReferenceCounted
	{
		IJoint *Target;
		IJoint *Effector;
		u16 Iteration;
		core::array<IJoint*> Chain;
		f32 RotationLimit;

	public:
		CIkResolver(IJoint *target, IJoint *effector,
				u16 length, u16 iteration, f32 weight);
		~CIkResolver();
		void resolve();
	};
	CIkResolver *addIkResolver(u16 target, u16 effector, u16 length,
			u16 iteration, f32 weight)
	{
	    if (weight < 0.000001)
            return 0;

		CIkResolver *ikResolver=
			new CIkResolver(Joints[target], Joints[effector],
					length, iteration, weight);
		IkResolvers.push_back(ikResolver);
		return ikResolver;
	}

	////////////////////////////////////////////////////////////
	// morphing
	////////////////////////////////////////////////////////////
	class CMorphing : public IReferenceCounted
	{
	public:
		CMorphing(const core::stringc &name);
		void morph(
				core::array<video::S3DVertex> &Vertices,
				const core::array<u16> &Indices);
		void reallocate(u16 size);
		void addIndexCoord(u16 index, const core::vector3df &pos);
		void assignCurve(SMorphLinearCurve *curve){ Curve=curve; }
		void setFrame(s32 frame);
		core::array<u16>& getIndices(){ return Indices; }
		core::stringc getName()const{ return Name; }

	private:
		core::stringc Name;
		core::array<u16> Indices;
		core::array<core::vector3df> Positions;
		SMorphLinearCurve *Curve;
		f32 CurrentWeight;
	};
	CMorphing* addMorphing(const core::stringc &name);
	CMorphing* getMorphingBase();

	////////////////////////////////////////////////////////////
	// bullet
	////////////////////////////////////////////////////////////
	bullet::CWorld* getBulletWorld(){ return BulletWorld; }

private:
	u32 CurrentFrame;
	bool isDeformed;
	// geometries
	core::aabbox3df BoundingBox;
	SMeshBuffer *VertexBuffer;
	core::array<IMeshBuffer*> MeshBuffers;
	// skinning
	core::array<f32> BoneWeights;
	core::array<u16> BoneIndices0;
	core::array<u16> BoneIndices1;
	SMeshBuffer *DeformedVertices;
	// joints
	IJoint *Root;
	core::array<IJoint*> Joints;
	// ik resolvers
	core::array<CIkResolver*> IkResolvers;
	// morphing
	core::array<CMorphing*> Morphings;
	CMorphing *MorphingBase;
	// motion
	CVMDCustomSkinMotion *Motion;
	// bullet
	bullet::CWorld *BulletWorld;
};

} /// namespace scene
} // namespace irr

#endif // CUSTOM_SKINNED_MESH_H_INCLUDED
