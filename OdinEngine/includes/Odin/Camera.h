#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL/SDL.h>

//Camera - 2D

namespace odin {
	class Camera
	{
	public:
		Camera();
		~Camera();

		void init(int screenWidth, int screenHeight);

		void update();

		void setPosition(const glm::vec2& newPosition, bool updateOriginalPos = false ) { 
			_position = newPosition; 
			_requiresMatrixUpdate = true; 
			if (updateOriginalPos)
				_origPosition = newPosition;
		}
		void setScale(float newScale) { _scale = newScale; _requiresMatrixUpdate = true; }

		glm::vec2 getPosition() { return _position; }
		glm::mat4 getCameraMatrix() { return _cameraMatrix; }
		float getScale() { return _scale; }

		void shake();

		bool   cinematic = false;
		bool   shaking = false;
	private:
		void continueShaking();

		glm::vec2 _position;
		glm::mat4 _cameraMatrix;
		glm::mat4 _orthoMatrix;
		float _scale;
		bool _requiresMatrixUpdate;
		int _screenWidth;
		int _screenHeight;

		//for automated motion
		glm::vec2 _origPosition;
		unsigned int _duration;
		unsigned int _startTime;
		float _magnitude;
	};
}
