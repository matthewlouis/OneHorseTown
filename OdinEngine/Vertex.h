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

struct UV {
	float u;
	float v;
};

struct Vertex {
	Position position; //xy coords for 2d
	Color color; //4 byte rgba color
	UV uv; //uv texture coords

	void setColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a) {
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;
	}

	void setPosition(float x, float y) {
		position.x = x;
		position.y = y;
	}

	void setUV(float u, float v) {
		uv.u = u;
		uv.v = v;
	}

	//initializes vertex in one line
	void init(GLubyte r, GLubyte g, GLubyte b, GLubyte a, float x, float y, float u, float v) {
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;
		position.x = x;
		position.y = y;
		uv.u = u;
		uv.v = v;
	}
};
