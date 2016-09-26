#pragma once

#include <unordered_map>

namespace OdinEngine {
	class InputManager
	{
	public:
		InputManager();
		~InputManager();

		void keyDown(unsigned int keyID);
		void keyUp(unsigned int keyID);

		bool isKeyPressed(unsigned int keyID);
	private:
		std::unordered_map<unsigned int, bool> _keyMap; //keeps track of pressed buttons
	};
}
