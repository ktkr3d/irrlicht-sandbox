#include "CSceneNodeAnimatorCameraRokuro.h"
#include <iostream>

namespace irr{
	namespace scene {

		CSceneNodeAnimatorCameraRokuro::CSceneNodeAnimatorCameraRokuro(
				gui::ICursorControl* cursor)
			: cursor_(cursor), head_(0), pitch_(0),
			distance_(50), Shift(core::matrix4::EM4CONST_IDENTITY)
			{
				is_mouse_down_[0]=false;
				is_mouse_down_[1]=false;
				is_mouse_down_[2]=false;
			}

		//! Destructor
		CSceneNodeAnimatorCameraRokuro::~CSceneNodeAnimatorCameraRokuro() {}

		core::matrix4 CSceneNodeAnimatorCameraRokuro::getMatrix(
				const core::vector3df &target)const
		{
			core::matrix4 r(core::matrix4::EM4CONST_IDENTITY);
			r.setRotationDegrees(core::vector3df(-pitch_, -head_, 0));

			core::matrix4 t(core::matrix4::EM4CONST_IDENTITY);
			t.setTranslation(target);

			return r*t;
		}

		//! Animates a scene node.
		void CSceneNodeAnimatorCameraRokuro::animateNode(
				ISceneNode* node, u32 timeMs)
		{
			if (!node || node->getType() != ESNT_CAMERA)
				return;

			// get camera
			ICameraSceneNode* camera = static_cast<ICameraSceneNode*>(node);

			// If the camera isn't the active camera, and receiving input, 
			// then don't process it.
			if(!camera->isInputReceiverEnabled())
				return;

			// inactive camera
			ISceneManager * smgr = camera->getSceneManager();
			if(smgr && smgr->getActiveCamera() != camera)
				return;

			// update camera
			core::vector3df pos(0, 0, -distance_);

			getMatrix(camera->getTarget()).transformVect(pos);

			camera->setPosition(pos);

			camera->setViewMatrixAffector(Shift);
		}

		//! Creates a clone of this animator.
		ISceneNodeAnimator* CSceneNodeAnimatorCameraRokuro::createClone(
				ISceneNode* node, ISceneManager* newManager)
		{
			return 0;
		}

		//! Returns true if this animator receives events.
		bool CSceneNodeAnimatorCameraRokuro::isEventReceiverEnabled() const
		{
			return true;
		}

		//! Event receiver,
		//! override this function for camera controlling animators
		bool CSceneNodeAnimatorCameraRokuro::OnEvent(const SEvent& event)
		{
			if (event.EventType != EET_MOUSE_INPUT_EVENT)
				return false;

			core::position2di mouse=cursor_->getPosition();
			switch(event.MouseInput.Event)
			{
			case EMIE_LMOUSE_PRESSED_DOWN:
				is_mouse_down_[0] = true;
				break;
			case EMIE_RMOUSE_PRESSED_DOWN:
				is_mouse_down_[2] = true;
				break;
			case EMIE_MMOUSE_PRESSED_DOWN:
				is_mouse_down_[1] = true;
				break;
			case EMIE_LMOUSE_LEFT_UP:
				is_mouse_down_[0] = false;
				break;
			case EMIE_RMOUSE_LEFT_UP:
				is_mouse_down_[2] = false;
				break;
			case EMIE_MMOUSE_LEFT_UP:
				is_mouse_down_[1] = false;
				break;
			case EMIE_MOUSE_MOVED:
				if(isMouseKeyDown(0)){
					dolly(static_cast<float>(mouse_.Y-mouse.Y));
				}
				if(isMouseKeyDown(1)){
					translate(mouse_, mouse);
				}
				if(isMouseKeyDown(2)){
					rotate(mouse_, mouse);
				}
				break;
			case EMIE_MOUSE_WHEEL:
				dolly(event.MouseInput.Wheel);
				break;
			case EMIE_COUNT:
				return false;
			default:
				std::cout << "unknown: " << event.MouseInput.Event << std::endl;
				break;
			}
			mouse_=mouse;
			return true;
		}

		//! Returns type of the scene node animator
		ESCENE_NODE_ANIMATOR_TYPE 
			CSceneNodeAnimatorCameraRokuro::getType() const
			{
				return ESNAT_UNKNOWN;
			}

		////////////////////////////////////////////////////////////
		// no virtual
		////////////////////////////////////////////////////////////
		void CSceneNodeAnimatorCameraRokuro::dolly(float factor)
		{
			if(factor<0){
				distance_*=1.1f;
			}
			else if(factor>0){
				distance_*=0.9f;
			}
		}

		void CSceneNodeAnimatorCameraRokuro::translate(
				const core::position2di &start, 
				const core::position2di &end)
		{
			Shift[12]+=(end.X-start.X);
			Shift[13]+=(start.Y-end.Y);
		}

		void CSceneNodeAnimatorCameraRokuro::rotate(
				const core::position2di &start, 
				const core::position2di &end)
		{
			head_+=(start.X-end.X);
			pitch_+=(start.Y-end.Y);
		}

		bool CSceneNodeAnimatorCameraRokuro::isMouseKeyDown(s32 key)
		{
			return is_mouse_down_[key];
		}
	}
}
