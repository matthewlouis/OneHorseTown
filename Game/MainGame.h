#pragma once

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <vector>

#include <OdinEngine/Sprite.h>
#include <OdinEngine/GLSLProgram.h>
#include <OdinEngine/GLTexture.h>
#include <OdinEngine/Window.h>
#include <OdinEngine/Camera.h>
#include <OdinEngine/SpriteBatch.h>
#include <OdinEngine/InputManager.h>
#include <OdinEngine/Timing.h>


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
 
	OdinEngine::Window _window;
	int			_screenWidth, _screenHeight;
	GameState   _gameState;

	OdinEngine::Camera _camera;
	OdinEngine::SpriteBatch _spriteBatch;
	OdinEngine::InputManager _inputManager; //handles input

	OdinEngine::GLSLProgram _colorShaderProgram;

	OdinEngine::FPSLimiter _fpsLimiter;

	float _fps;
	float _time;
};

