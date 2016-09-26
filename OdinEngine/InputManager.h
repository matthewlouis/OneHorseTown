#pragma once

#include <unordered_map>

namespace OdinEngine {
	class InputManager
	{
	public:
		InputManager();
		~InputManager();

		void update();

		void keyDown(unsigned int keyID);
		void keyUp(unsigned int keyID);

		//returns true if key is held down
		bool isKeyDown(unsigned int keyID);

		//returns true if key was pressed this frame and not last
		bool isKeyPressed(unsigned int keyID);
	private:
		bool wasKeyDown(unsigned int keyID); //return true if key was down in previous frame

		std::unordered_map<unsigned int, bool> _keyMap; //keeps track of down buttons
		std::unordered_map<unsigned int, bool> _previousKeyMap;
	};
}
