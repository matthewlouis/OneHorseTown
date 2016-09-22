#include "MainGame.h"
#include "GameConstants.h"
#include "Errors.h"

#include <iostream>
#include <string>


MainGame::MainGame()
{
	_window = nullptr;
	_screenWidth = constants::WINDOW_WIDTH;
	_screenHeight = constants::WINDOW_HEIGHT;
	_gameState = GameState::PLAY;
}


MainGame::~MainGame()
{
}

void MainGame::run() {
	initSystems();
	
	_sprite.init(-1.0f, -1.0f, 1.0f, 1.0f);

	gameLoop();
}

void MainGame::initSystems() {
	//Initialize SDL
	SDL_Init(SDL_INIT_EVERYTHING);

	//Create OpenGL window
	//SDL_WINDOWPOS_CENTERED creates window in the center position using given width/height
	_window = SDL_CreateWindow(constants::NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
								_screenWidth, _screenHeight, SDL_WINDOW_OPENGL);

	//simple error checking
	if (_window == nullptr) {
		fatalError("SDL Window could not be created.");
	}

	SDL_GLContext glContext = SDL_GL_CreateContext(_window);
	if (glContext == nullptr) {
		fatalError("SDL_GL context could not be created.");
	}

	GLenum error = glewInit();
	if (error != GLEW_OK) {
		fatalError("Could not init glew.");
	}

	//enable double buffering for a smoooooth experience.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	glClearColor(0.5f, 0, 0, 1); //set clear color to red

	initShaders();
}

void MainGame::initShaders() {
	_colorShaderProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
	_colorShaderProgram.addAttribute("vertexPosition");
	_colorShaderProgram.linkShaders();
}

void MainGame::gameLoop() {

	while (_gameState != GameState::EXIT) {
		processInput();
		drawGame();
	}
}

void MainGame::processInput() {

	//check what the fack players have been doing
	SDL_Event event;

	//while the players have actually mashed some buttons or keys
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_CONTROLLERAXISMOTION:
			//handle controller joystick movement
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			//handle controller button down
			break;
		case SDL_CONTROLLERBUTTONUP:
			//handle controller button up
			break;
		case SDL_MOUSEMOTION:
			std::cout << event.motion.x << " " << event.motion.y << '\n';
			break;
		case SDL_QUIT:
			_gameState = GameState::EXIT;
		}
	}
}

void MainGame::drawGame() {
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_colorShaderProgram.use();

	_sprite.draw();

	_colorShaderProgram.unUse();

	SDL_GL_SwapWindow(_window); //swaps buffer
}
