#include "MainGame.h"
#include "GameConstants.h"
#include <OdinEngine/Errors.h>
#include <OdinEngine/OdinEngine.h>
#include <OdinEngine/ResourceManager.h>
#include <iostream>
#include <string>


MainGame::MainGame() : 
	_screenWidth(constants::WINDOW_WIDTH),
	_screenHeight(constants::WINDOW_HEIGHT),
	_gameState(GameState::PLAY)
{
	_camera.init(_screenWidth, _screenHeight); //init our camera
}


MainGame::~MainGame()
{
}

void MainGame::run() {
	initSystems();

	gameLoop();
}

void MainGame::initSystems() {

	OdinEngine::init();

	_window.create("Spaghetti Western: WORKING TITLE :P", constants::WINDOW_WIDTH, constants::WINDOW_HEIGHT, 0);

	initShaders();

	_spriteBatch.init();
	_fpsLimiter.init(constants::FPS_LIMIT);
}

void MainGame::initShaders() {
	_colorShaderProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
	_colorShaderProgram.addAttribute("vertexPosition");
	_colorShaderProgram.addAttribute("vertexColor");
	_colorShaderProgram.addAttribute("vertexUV");
	_colorShaderProgram.linkShaders();
}

void MainGame::gameLoop() {

	while (_gameState != GameState::EXIT) {

		_fpsLimiter.beginFrame();

		processInput();

		_camera.update();

		drawGame();

		_fps = _fpsLimiter.endFrame();

		//DEBUG: print FPS every ten frames
		static int frameCounter = 0;
		++frameCounter;
		if (frameCounter == 10) {
			printf("FPS: %f\n", _fps);
			frameCounter = 0;
		}
	}
}

void MainGame::processInput() {

	//check what the fack players have been doing
	SDL_Event event;

	static const float CAMERA_SPEED = 2.0f;
	static const float SCALE_SPEED  = 0.1f;

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
			//mouse movement - probably not needed
			break;
		case SDL_KEYDOWN:
			//key press
			_inputManager.keyDown(event.key.keysym.sym);
			break;
		case SDL_KEYUP:
			//key press
			_inputManager.keyUp(event.key.keysym.sym);
			break;
		case SDL_QUIT:
			_gameState = GameState::EXIT;
		}



		if (_inputManager.isKeyPressed(SDLK_w)) {
			_camera.setPosition(_camera.getPosition() + glm::vec2(0.0f, CAMERA_SPEED));
		}
		if (_inputManager.isKeyPressed(SDLK_s)) {
			_camera.setPosition(_camera.getPosition() + glm::vec2(0.0f, -CAMERA_SPEED));
		}
		if (_inputManager.isKeyPressed(SDLK_a)) {
			_camera.setPosition(_camera.getPosition() + glm::vec2(-CAMERA_SPEED, 0.0f));
		}
		if (_inputManager.isKeyPressed(SDLK_d)) {
			_camera.setPosition(_camera.getPosition() + glm::vec2(CAMERA_SPEED, 0.0f));
		}
		if (_inputManager.isKeyPressed(SDLK_q)) {
			_camera.setScale(_camera.getScale() + SCALE_SPEED);
		}
		if (_inputManager.isKeyPressed(SDLK_e)) {
			_camera.setScale(_camera.getScale() - SCALE_SPEED);
		}
	}
}

void MainGame::drawGame() {
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_colorShaderProgram.use();
	
	//set active texture 
	glActiveTexture(GL_TEXTURE0);
	GLint textureLocation = _colorShaderProgram.getUniformLocation("textureSampler");
	glUniform1i(textureLocation, 0);

	//Set the camera matrix
	GLint pLocation = _colorShaderProgram.getUniformLocation("P");
	glm::mat4 cameraMatrix = _camera.getCameraMatrix();
	glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(cameraMatrix[0][0]));
	
	_spriteBatch.begin();

	//creating sprites for testing
	glm::vec4 uv(0.0f, 0.0f, 1.0f, 1.0f);
	glm::vec4 pos(0.0f, 0.0f, 60.0f, 140.0f);
	static OdinEngine::GLTexture texture = OdinEngine::ResourceManager::getTexture("Textures/pixel_cowboy.png");

	_spriteBatch.draw(pos, uv, texture.id, 0.0f, OdinEngine::Color());
	_spriteBatch.draw(pos + glm::vec4(200.0f, 0.0f, 0.0f, 0.0f), uv * glm::vec4(1, 1, -1, 1), texture.id, 0.0f, OdinEngine::Color());
	
	_spriteBatch.end();
	_spriteBatch.renderBatch();

	//unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
	_colorShaderProgram.unUse();

	_window.swapBuffer();
}


