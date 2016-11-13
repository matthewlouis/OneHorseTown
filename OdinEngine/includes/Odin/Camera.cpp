#include "Camera.h"
#include <SDL/SDL.h>
#include <stdio.h>


namespace odin {
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
		if (shaking)
			continueShaking();

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

	void Camera::shake()
	{
		
		_duration = 100;
		_magnitude = 1.0f;

		_startTime = SDL_GetTicks();

		glm::vec2 originalCamPos = _position;

		shaking = true;

		continueShaking();
	}

	void Camera::continueShaking()
	{
		float magnitude = 2.0f; //hard coding magnitude for now
		unsigned int elapsed = SDL_GetTicks() - _startTime;

		if (SDL_GetTicks() - _startTime < _duration) {

			float percentComplete = elapsed / (float)_duration;

			//clamp value
			float value = 4.0f * percentComplete - 3.0f;
			value = value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : value;;

			float damper = 1.0f - value;

			// map value to [-1, 1]
			float x = rand() % 2 * 2.0f - 1.0f;
			float y = rand() % 2 * 2.0f - 1.0f;
			x *= magnitude * damper;
			y *= magnitude * damper;

			setPosition(glm::vec2(x, y));
		}
		else {
			shaking = false;
			setPosition(_origPosition);
		}
	}
}