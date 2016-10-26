#pragma once

const int PIXEL_SIZE = 4;

// sega genesis resolution 320x224
const float VIRTUAL_WIDTH = 256;
const float VIRTUAL_HEIGHT = 160;

// Screen width.
const int WIDTH = int( VIRTUAL_WIDTH * PIXEL_SIZE );

// Screen height.
const int HEIGHT = int( VIRTUAL_HEIGHT * PIXEL_SIZE );

// OpenGL draw scaling.
const int LOW_WIDTH = int( WIDTH / PIXEL_SIZE );
const int LOW_HEIGHT = int( HEIGHT / PIXEL_SIZE );

const float SCALE = VIRTUAL_WIDTH / 2;

enum AXIS_DIRECTION { N, NE, E, SE, S, SW, W, NW };