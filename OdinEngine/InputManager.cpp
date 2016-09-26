#include "InputManager.h"


namespace OdinEngine {
	InputManager::InputManager()
	{
	}


	InputManager::~InputManager()
	{
	}

	void InputManager::keyDown(unsigned int keyID) {
		_keyMap[keyID] = true;
	}

	void InputManager::keyUp(unsigned int keyID) {
		_keyMap[keyID] = false;
	}

	bool InputManager::isKeyPressed(unsigned int keyID) {
		auto it = _keyMap.find(keyID);
		if (it != _keyMap.end()) {
			return it->second; //return boolean value
		}else
		{
			return false;
		}
	}
}

