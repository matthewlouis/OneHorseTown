#pragma once

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <vector>

#include <OdinEngine/Sprite.h>
#include <OdinEngine/GLSLProgram.h>
#include <OdinEngine/GLTexture.h>
#include <OdinEngine/Window.h>


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

	OdinEngine::Window _window;
	int			_screenWidth, _screenHeight;
	GameState   _gameState;

	std::vector <OdinEngine::Sprite*> _sprites;

	OdinEngine::GLSLProgram _colorShaderProgram;

	//fps variables
	float _fps;
	float _frameTime;

	float _time;
};

