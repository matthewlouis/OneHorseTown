#include "Errors.h"

#include <cstdlib>

#include <iostream>
#include <SDL/SDL.h>
#include <FMOD/fmod.h>

namespace OdinEngine {
	void fatalError(std::string errorString)
	{
		std::cout << errorString << std::endl;
		std::cout << "Enter any key to quit...";
		int tmp;
		std::cin >> tmp;
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	int fmodErrorCheck(FMOD_RESULT result) {
		if (result != FMOD_OK) {
			return 1; //everything worked
		}
	}
}