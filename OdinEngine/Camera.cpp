#include "Camera.h"


namespace OdinEngine {
	Camera::Camera() :
		_position(0.0f, 0.0f),
		_cameraMatrix(1.0f),
		_orthoMatrix(1.0f),
		_scale(1.0f),
		_requiresMatrixUpdate(1),
		_screenWidth(640), _screenHeight(480)
	{
	}


	Camera::~Camera()
	{
	}

	void Camera::init(int screenWidth, int screenHeight) {
		_screenWidth = screenWidth;
		_screenHeight = screenHeight;
		_orthoMatrix = glm::ortho(0.0f, (float)_screenWidth, 0.0f, (float)_screenHeight);
	}


	void Camera::update() {
		if (_requiresMatrixUpdate) { //saves us from updating camera if we don't need to

			// move in opposite direction(if camera moves left obj appear to move right)
			// using screen width/height to center camera focal point
			glm::vec3 translate(-_position.x + _screenWidth/2, -_position.y + _screenHeight/2, 0.0f);
			_cameraMatrix = glm::translate(_orthoMatrix, translate); //translate


			glm::vec3 scale(_scale, _scale, 0.0f); //create scale vector	
			_cameraMatrix = glm::scale(_cameraMatrix, scale); //apply scale to matrix

			//matrix no longer needs updating now that calculations have been performed
			_requiresMatrixUpdate = false;
		}
	}
}