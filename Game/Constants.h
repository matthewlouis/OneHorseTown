#pragma once

int PIXEL_SIZE = 2;

// sega genesis resolution 320x224
const float VIRTUAL_WIDTH = 480;
const float VIRTUAL_HEIGHT = 288;

// Screen width.
int WIDTH = int( VIRTUAL_WIDTH * PIXEL_SIZE );

// Screen height.
int HEIGHT = int( VIRTUAL_HEIGHT * PIXEL_SIZE );

// OpenGL draw scaling.
int LOW_WIDTH = int( WIDTH / PIXEL_SIZE );
int LOW_HEIGHT = int( HEIGHT / PIXEL_SIZE );

const float SCALE = VIRTUAL_WIDTH / 2;

enum AXIS_DIRECTION { N, NE, E, SE, S, SW, W, NW };

const int MAX_PLAYERS = 4;

const float CAMERA_MOVE_AMOUNT = 20.0f;