#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//Camera - only 2D for now.

namespace OdinEngine {
	class Camera
	{
	public:
		Camera();
		~Camera();

		void init(int screenWidth, int screenHeight);

		void update();

		void setPosition(const glm::vec2& newPosition) { _position = newPosition; _requiresMatrixUpdate = true; }
		void setScale(float newScale) { _scale = newScale; _requiresMatrixUpdate = true; }

		glm::vec2 getPosition() { return _position; }
		glm::mat4 getCameraMatrix() { return _cameraMatrix; }
		float getScale() { return _scale; }

	private:
		glm::vec2 _position;
		glm::mat4 _cameraMatrix;
		glm::mat4 _orthoMatrix;
		float _scale;
		bool _requiresMatrixUpdate;
		int _screenWidth;
		int _screenHeight;
	};
}
