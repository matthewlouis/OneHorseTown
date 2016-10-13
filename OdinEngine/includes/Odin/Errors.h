#pragma once

#include <string>
#include <FMOD/fmod.h>

namespace OdinEngine {
	//call if error is game-breaking and need to output info to console
	extern void fatalError(std::string errorString);

	//used for confirming status of fmod calls
	extern int fmodErrorCheck(FMOD_RESULT result);
}
