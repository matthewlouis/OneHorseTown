#pragma once

const int PIXEL_SIZE = 4;

// sega genesis resolution 320x224
const float VIRTUAL_WIDTH = 320;
const float VIRTUAL_HEIGHT = 224;

// Screen width.
const int WIDTH = VIRTUAL_WIDTH * PIXEL_SIZE;

// Screen height.
const int HEIGHT = VIRTUAL_HEIGHT * PIXEL_SIZE;

// OpenGL draw scaling.
const int LOW_WIDTH = WIDTH / PIXEL_SIZE;
const int LOW_HEIGHT = HEIGHT / PIXEL_SIZE;

const float SCALE = 11.5;

enum AXIS_DIRECTION { N, NE, E, SE, S, SW, W, NW };