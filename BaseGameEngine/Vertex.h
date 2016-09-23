#pragma once
#include <GL/glew.h>
/**
/** Contains all info needed for a lonesome vertex.
**/

struct Vertex {
	struct Position {
		float x;
		float y;
	} position;

	struct Color {
		GLubyte r;
		GLubyte g;
		GLubyte b;
		GLubyte a;
	} color;
};
