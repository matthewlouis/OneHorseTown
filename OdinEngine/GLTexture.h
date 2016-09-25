#pragma once
#include <GL/glew.h>

// GLTexture.h
// Contains all info required for an openGL texture

namespace OdinEngine {
	struct GLTexture {
		GLuint id;
		int width;
		int height;
	};
}