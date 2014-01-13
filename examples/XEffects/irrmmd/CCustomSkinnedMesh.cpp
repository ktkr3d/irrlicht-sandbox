#include "CCustomSkinnedMesh.h"
#include "CVMDCustomSkinMotion.h"
#include "SRotPosKey.h"
//#include "btJoint.h"
#include "CJoint.h"
#include "irrbullet.h"
#include <cassert>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>

namespace irr {

static f32 clamp(f32 v, f32 min, f32 max)
{
	return std::min(max, std::max(min, v));
}

namespace scene {

static const f32 EPSILON=1e-5f;
static const u32 FPMS=1000/30;

///////////////////////////////////////////////////////////////////////////////
// CCustomSkinnedMesh
///////////////////////////////////////////////////////////////////////////////
CCustomSkinnedMesh::CCustomSkinnedMesh()
	: VertexBuffer(new SMeshBuffer), DeformedVertices(new SMeshBuffer),
	Root(new CJoint("__root__", core::vector3df(0, 0, 0))), Motion(0),
	CurrentFrame(-1), isDeformed(false), MorphingBase(0),
	BulletWorld(new bullet::CWorld)
{
}

CCustomSkinnedMesh::~CCustomSkinnedMesh()
{
	BulletWorld->drop();
	BulletWorld=0;

	VertexBuffer->drop();
	VertexBuffer=0;
	DeformedVertices->drop();
	DeformedVertices=0;
	for(u32 i=0; i<MeshBuffers.size(); ++i){
		MeshBuffers[i]->drop();
	}
	MeshBuffers.clear();

	Root->drop();
	Root=0;
	for(u32 i=0; i<Joints.size(); ++i){
		Joints[i]->drop();
	}
	Joints.clear();

	if(Motion){
		Motion->grab();
		Motion=0;
	}

	for(u32 i=0; i<IkResolvers.size(); ++i){
		IkResolvers[i]->drop();
	}
	IkResolvers.clear();

	if(MorphingBase){
		MorphingBase->drop();
		MorphingBase=0;
	}
	for(u32 i=0; i<Morphings.size(); ++i){
		Morphings[i]->drop();
	}
	Morphings.clear();
}

u32 CCustomSkinnedMesh::getMeshBufferCount(void) const
{
	return MeshBuffers.size();
}

IMeshBuffer *CCustomSkinnedMesh::getMeshBuffer(const video::SMaterial &material) const
{
	for(u32 i=0; i<MeshBuffers.size(); ++i){
		if(MeshBuffers[i]->getMaterial()==material){
			return MeshBuffers[i];
		}
	}
	return 0;
}

IMeshBuffer *CCustomSkinnedMesh::getMeshBuffer(u32 nr) const
{
	return MeshBuffers[nr];
}

const core::aabbox3df &CCustomSkinnedMesh::getBoundingBox(void) const
{
	return BoundingBox;
}

void CCustomSkinnedMesh::setBoundingBox(const core::aabbox3df &bbox)
{
	BoundingBox=bbox;
}

void CCustomSkinnedMesh::setMaterialFlag(video::E_MATERIAL_FLAG,bool)
{
	assert(false);
}

void CCustomSkinnedMesh::setHardwareMappingHint(E_HARDWARE_MAPPING,E_BUFFER_TYPE)
{
	assert(false);
}

void CCustomSkinnedMesh::setDirty(E_BUFFER_TYPE buffer)
{
	for (u32 i=0; i<MeshBuffers.size(); ++i)
		MeshBuffers[i]->setDirty(buffer);
}

u32 CCustomSkinnedMesh::getFrameCount(void) const
{
	if(Motion){
		return Motion->getFrameCount();
	}
	else{
		return 0;
	}
}

f32 CCustomSkinnedMesh::getAnimationSpeed() const
{
	return 0;
}

void CCustomSkinnedMesh::setAnimationSpeed(f32 fps)
{

}

IMesh *CCustomSkinnedMesh::getMesh(s32 frame, s32, s32, s32)
{
	if(Motion){
		if(!isDeformed || frame!=CurrentFrame){
			setFrame(frame);
		}
	}
	else{
		if(!isDeformed){
			// copy
			DeformedVertices->Vertices=VertexBuffer->Vertices;
			isDeformed=true;
			setDirty(EBT_VERTEX);
		}
	}

	return this;
}

void CCustomSkinnedMesh::setFrame(u32 frame)
{
	// update bone
	updateBone(frame);

	// update physics
	BulletWorld->update((frame-CurrentFrame)*FPMS);

	// deform
	deform(frame);

	isDeformed=true;
	setDirty(EBT_VERTEX);
	CurrentFrame=frame;
}

void CCustomSkinnedMesh::updateBone(u32 frame)
{
	// update bones
	for(u32 i=0; i<Joints.size(); ++i){
		Joints[i]->setFrame(frame);
	}
	Root->updateRecursive();

	// resolve IK
	for(u32 i=0; i<IkResolvers.size(); ++i){
		IkResolvers[i]->resolve();
	}
}

void CCustomSkinnedMesh::deform(u32 frame)
{
	// copy
	DeformedVertices->Vertices=VertexBuffer->Vertices;

	// morphing
	for(u32 i=0; i<Morphings.size(); ++i){
		CMorphing *morphing=Morphings[i];
		morphing->setFrame(CurrentFrame);
		morphing->morph(
				DeformedVertices->Vertices, MorphingBase->getIndices());
	}

	// update matrix
	for(size_t i=0; i<Joints.size(); ++i){
		Joints[i]->calcSkinning();
	}

	// skinning
	for(size_t i=0; i<DeformedVertices->Vertices.size(); ++i){
		f32 weight=BoneWeights[i];
		if(weight>1.0f-EPSILON){
			// use bone0
			core::vector3df transformed;
			Joints[BoneIndices0[i]]->getSkinning().transformVect(
					transformed,
					DeformedVertices->Vertices[i].Pos);
			DeformedVertices->Vertices[i].Pos=transformed;
		}
		else if(weight<EPSILON){
			// use bone1
			core::vector3df transformed;
			Joints[BoneIndices1[i]]->getSkinning().transformVect(
					transformed,
					DeformedVertices->Vertices[i].Pos);
			DeformedVertices->Vertices[i].Pos=transformed;
		}
		else{
			// bone0
			core::vector3df v0;
			Joints[BoneIndices0[i]]->getSkinning().transformVect(
					v0,
					DeformedVertices->Vertices[i].Pos);
			// bone1
			core::vector3df v1;
			Joints[BoneIndices1[i]]->getSkinning().transformVect(
					v1,
					DeformedVertices->Vertices[i].Pos);
			// blend
			DeformedVertices->Vertices[i].Pos=v0*weight + v1*(1.0f-weight);
		}
	}

}

void CCustomSkinnedMesh::recalculateBoundingBox()
{
	if(VertexBuffer){
		BoundingBox=VertexBuffer->getBoundingBox();
	}
}

///////////////////////////////////////////////////////////////////////////////
// own interface
///////////////////////////////////////////////////////////////////////////////
SSharedMeshBuffer* CCustomSkinnedMesh::addIndexBuffer()
{
	SSharedMeshBuffer *indexBuffer=
		new SSharedMeshBuffer(&DeformedVertices->Vertices);
	MeshBuffers.push_back(indexBuffer);
	return indexBuffer;
}

void CCustomSkinnedMesh::addVertex(const video::S3DVertex &vertex,
		f32 weight, u16 bone0, u16 bone1)
{
	VertexBuffer->Vertices.push_back(vertex);
	BoneWeights.push_back(weight);
	BoneIndices0.push_back(bone0);
	BoneIndices1.push_back(bone1);
	DeformedVertices->Vertices.push_back(vertex);
}

void CCustomSkinnedMesh::reallocate(u16 size)
{
	VertexBuffer->Vertices.reallocate(size);
	BoneWeights.reallocate(size);
	BoneIndices0.reallocate(size);
	BoneIndices1.reallocate(size);
	DeformedVertices->Vertices.reallocate(size);
}

void CCustomSkinnedMesh::setVertexColor(u16 index, const video::SColor &color)
{
	VertexBuffer->Vertices[index].Color=color;
}

void CCustomSkinnedMesh::setMotion(CVMDCustomSkinMotion *motion)
{
	motion->grab();
	Motion=motion;

	// assign curve to each joint
	for(u32 i=0; i<Joints.size(); ++i){
		IJoint *joint=Joints[i];
		joint->assignCurve(Motion->getBoneCurve(joint->getName()));
	}

	// assign curve to each morph
	for(u32 i=0; i<Morphings.size(); ++i){
		CMorphing *morphing=Morphings[i];
		morphing->assignCurve(Motion->getMorphingCurve(morphing->getName()));
	}

	// set meshbuffer setting
	for(u32 i=0; i<MeshBuffers.size(); ++i){
		//MeshBuffers[i]->setHardwareMappingHint(EHM_STREAM);
		MeshBuffers[i]->setHardwareMappingHint(EHM_NEVER);
	}

	// initialize
	setFrame(0);
	//BulletWorld->syncBone();
	/*
	for(int i=0; i<1000; ++i){
		BulletWorld->update(100);
	}
	*/
}

IJoint* CCustomSkinnedMesh::addJoint(
		const core::stringc &name, const core::vector3df &pos)
{
	Joints.push_back(new CJoint(name, pos));
	return Joints.getLast();
}

///////////////////////////////////////////////////////////////////////////////
// ik resolver
///////////////////////////////////////////////////////////////////////////////
static core::quaternion constraint(const core::quaternion q)
{
	core::vector3df euler;
	q.toEuler(euler);
	// Xの回転角度の制限
	euler.X=clamp(euler.X, static_cast<f32>(-M_PI), -0.002f);
	// Xの回転のみのクォータニオンを作る
	float radianX=euler.X * 0.5f;
	float sinX = sinf(radianX);
	float cosX = cosf(radianX);
	return core::quaternion(sinX, 0, 0, cosX);
}

CCustomSkinnedMesh::CIkResolver::CIkResolver(IJoint *target, IJoint *effector,
		u16 length, u16 iteration, f32 weight)
: Iteration(iteration)
{
	RotationLimit=static_cast<f32>(weight * M_PI);

	target->grab();
	Target=target;

	effector->grab();
	Effector=effector;

	IJoint *joint=effector->getParent();
	for(u32 i=0; i<length; ++i, joint=joint->getParent()){
		joint->grab();
		Chain.push_back(joint);
		if(joint->getName()=="左ひざ" || joint->getName()=="右ひざ"){
			// 回転角度の制限
			joint->setConstraint(true);
		}
	}
}

CCustomSkinnedMesh::CIkResolver::~CIkResolver()
{
	Target->drop();
	Target=0;

	Effector->drop();
	Effector=0;

	for(u32 i=0; i<Chain.size(); ++i){
		Chain[i]->drop();
	}
	Chain.clear();
}

//! resolve CCD-IK
void CCustomSkinnedMesh::CIkResolver::resolve()
{
	core::vector3df targetPos=Target->getAccumulatedPosition();

	for(u32 i=0; i<Iteration; ++i){
		for(u32 j=0; j<Chain.size(); ++j){
			IJoint *joint=Chain[j];
			// 対象IK jointのローカル座標に変換
			core::matrix4 inverse;
			bool result=joint->getAccumulation().getInversePrimitive(inverse);
			assert(result);
			// target position in joint coordinate
			core::vector3df localTarget=targetPos;
			inverse.transformVect(localTarget);
			// effector position in joint coordinate
			core::vector3df localEffector=Effector->getAccumulatedPosition();
			inverse.transformVect(localEffector);

			if((localTarget-localEffector).getLengthSQ()<EPSILON){
				// 近くに来たら終了
				return;
			}

			// 方向ベクトル化
			localEffector.normalize();
			localTarget.normalize();

			// effector -> targetへの回転量
			f32 radAngle=static_cast<f32>(
					acos(localEffector.dotProduct(localTarget)));
			if(-EPSILON<radAngle && radAngle< EPSILON){
				continue;
			}
			radAngle=clamp(radAngle, -RotationLimit, RotationLimit);

			// 回転軸
			core::vector3df axis=localEffector.crossProduct(localTarget);
			if(axis.getLengthSQ()<EPSILON){
				continue;
			}
			axis.normalize();

			// 回転
			core::quaternion rotation;
			rotation.fromAngleAxis(radAngle, axis);
			if(joint->getConstraint()){
				rotation=constraint(rotation);
			}
			joint->setCurrentRotation(
					rotation * joint->getCurrentRotation()
					);

			// update current rotation and position
			for(int k=j ;k>=0 ; k--){
				Chain[k]->update();
			}
			Effector->update();
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
// morphing
///////////////////////////////////////////////////////////////////////////////
CCustomSkinnedMesh::CMorphing::CMorphing(const core::stringc &name)
	: Name(name), Curve(0), CurrentWeight(0)
{
}

void CCustomSkinnedMesh::CMorphing::morph(
		core::array<video::S3DVertex> &Vertices,
		const core::array<u16> &BaseIndices)
{
	if(CurrentWeight==0){
		return;
	}
	for(u32 i=0; i<Indices.size(); ++i){
		video::S3DVertex &v=Vertices[BaseIndices[Indices[i]]];
		v.Pos+=Positions[i]*CurrentWeight;
	}
}

void CCustomSkinnedMesh::CMorphing::reallocate(u16 size)
{
	Indices.reallocate(size);
	Positions.reallocate(size);
}

void CCustomSkinnedMesh::CMorphing::addIndexCoord(
		u16 index, const core::vector3df &pos)
{
	Indices.push_back(index);
	Positions.push_back(pos);
}

void CCustomSkinnedMesh::CMorphing::setFrame(s32 frame)
{
	if(Curve){
		CurrentWeight=Curve->getKey(frame);
	}
	else{
		CurrentWeight=0;
	}
}

CCustomSkinnedMesh::CMorphing*
CCustomSkinnedMesh::addMorphing(const core::stringc &name)
{
	CMorphing *morphing=new CMorphing(name);
	Morphings.push_back(morphing);
	return morphing;
}

CCustomSkinnedMesh::CMorphing*
CCustomSkinnedMesh::getMorphingBase()
{
	if(!MorphingBase){
		MorphingBase=new CMorphing("Basic");
	}
	return MorphingBase;
}

} // namespace scene
} // namespace irr
