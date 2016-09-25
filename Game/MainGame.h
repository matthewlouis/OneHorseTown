#pragma once

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <vector>

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
	void calcualteFPS();

	SDL_Window* _window;
	int			_screenWidth, _screenHeight;
	GameState   _gameState;

	std::vector <Sprite*> _sprites;

	GLSLProgram _colorShaderProgram;

	//fps variables
	float _fps;
	float _frameTime;

	float _time;
};

