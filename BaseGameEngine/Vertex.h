#pragma once
#include <GL/glew.h>
/**
/** Contains all info needed for a lonesome vertex.
**/

struct Position {
	float x;
	float y;
};

struct Color {
	GLubyte r;
	GLubyte g;
	GLubyte b;
	GLubyte a;
};

struct Vertex {
	Position position;

	Color color;
};
