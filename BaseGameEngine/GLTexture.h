#pragma once
#include <GL/glew.h>

// GLTexture.h
// Contains all info required for an openGL texture

struct GLTexture {
	GLuint id;
	int width;
	int height;
};