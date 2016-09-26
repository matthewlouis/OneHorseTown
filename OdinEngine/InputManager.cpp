#include "InputManager.h"


namespace OdinEngine {
	InputManager::InputManager()
	{
	}


	InputManager::~InputManager()
	{
	}

	void InputManager::update() {

		//loop through keyMap keys
		for each(auto& it in _keyMap) {
			_previousKeyMap[it.first] = it.second; //copy over to previous
		}
	}

	void InputManager::keyDown(unsigned int keyID) {
		_keyMap[keyID] = true;
	}

	void InputManager::keyUp(unsigned int keyID) {
		_keyMap[keyID] = false;
	}

	bool InputManager::isKeyDown(unsigned int keyID) {
		auto it = _keyMap.find(keyID);
		if (it != _keyMap.end()) {
			return it->second; //return boolean value
		}else
		{
			return false;
		}
	}

	bool InputManager::wasKeyDown(unsigned int keyID) {
		auto it = _previousKeyMap.find(keyID);
		if (it != _previousKeyMap.end()) {
			return it->second; //return boolean value
		}
		else
		{
			return false;
		}
	}

	bool InputManager::isKeyPressed(unsigned int keyID) {
		if (isKeyDown(keyID) && !wasKeyDown(keyID)) {
			return true;
		}else {
			return false;
		}
	}
}

