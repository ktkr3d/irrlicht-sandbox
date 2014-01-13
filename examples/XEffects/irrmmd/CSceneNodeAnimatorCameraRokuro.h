#ifndef ROKURO_CAMERA_H
#define ROKURO_CAMERA_H

#include <irrlicht.h>

namespace irr{
	namespace scene {
		//////////////////////////////////////////////////////////// 
		// custom camera
		//////////////////////////////////////////////////////////// 
		class CSceneNodeAnimatorCameraRokuro : public ISceneNodeAnimator
		{
			bool is_mouse_down_[3];
			gui::ICursorControl *cursor_;
			core::position2di mouse_;
			float head_;
			float pitch_;
			float distance_;
			core::matrix4 Shift;

		public:
			CSceneNodeAnimatorCameraRokuro(gui::ICursorControl* cursor);

			//! Destructor
			virtual ~CSceneNodeAnimatorCameraRokuro();

			//! Animates a scene node.
			virtual void animateNode(ISceneNode* node, u32 timeMs);

			//! Creates a clone of this animator.
			virtual ISceneNodeAnimator* createClone(
					ISceneNode* node, ISceneManager* newManager=0);

			//! Returns true if this animator receives events.
			virtual bool isEventReceiverEnabled() const;

			//! Event receiver,
			//! override this function for camera controlling animators
			virtual bool OnEvent(const SEvent& event);

			//! Returns type of the scene node animator
			virtual ESCENE_NODE_ANIMATOR_TYPE getType() const;

			////////////////////////////////////////////////////////////
			// no virtual
			////////////////////////////////////////////////////////////
			void dolly(float factor);

			void setDistance(float distance){ distance_=distance; }

			void translate(
					const core::position2di &start, 
					const core::position2di &end);

			void rotate(
					const core::position2di &start, 
					const core::position2di &end);

			bool isMouseKeyDown(s32 key);

			core::matrix4 getMatrix(const core::vector3df &target)const;
		};
	} // namespace scene
} // namespace irr

#endif // ROKURO_CAMERA_H
