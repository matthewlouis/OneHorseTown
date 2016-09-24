#pragma once

#include <SDL/SDL.h>
#include <GL/glew.h>

#include "Sprite.h"
#include "GLSLProgram.h"
#include "GLTexture.h"


enum class GameState {PLAY, PAUSE, EXIT};

class MainGame
{
public:
	MainGame();
	~MainGame();

	void run();
private:
	void initSystems();
	void initShaders();
	void gameLoop();
	void processInput();
	void drawGame();

	SDL_Window* _window;
	int			_screenWidth, _screenHeight;
	GameState   _gameState;

	Sprite _sprite;
	GLTexture _playerTexture; //temp texture testing

	GLSLProgram _colorShaderProgram;

	float _time;
};

