#include <SDL/SDL.h>
#include <GL/glew.h>

#include "OdinEngine.h"

namespace OdinEngine {
	int init() {
		//Initialize SDL
		SDL_Init(SDL_INIT_EVERYTHING);

		//enable double buffering for a smoooooth experience.
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		return 0;
	}
}