#include "MainGame.h"
#include "GameConstants.h"
#include <OdinEngine/Errors.h>
#include <OdinEngine/OdinEngine.h>
#include <iostream>
#include <string>


MainGame::MainGame() : 
	_screenWidth(constants::WINDOW_WIDTH),
	_screenHeight(constants::WINDOW_HEIGHT),
	_gameState(GameState::PLAY),
	_time(0)
{
	_camera.init(_screenWidth, _screenHeight); //init our camera
}


MainGame::~MainGame()
{
}

void MainGame::run() {
	initSystems();
	
	//create cowboy sprite
	_sprites.push_back(new OdinEngine::Sprite());
	_sprites.back()->init(0.0f, 0.0f, _screenWidth/2, _screenWidth / 2, "Textures/pixel_cowboy.png");

	//create horse sprite
	_sprites.push_back(new OdinEngine::Sprite());
	_sprites.back()->init(_screenWidth / 2, 0.0f, _screenWidth / 2, _screenWidth / 2, "Textures/pixel_cowboy.png");

	gameLoop();
}

void MainGame::initSystems() {

	OdinEngine::init();

	_window.create("Spaghetti Western: WORKING TITLE :P", constants::WINDOW_WIDTH, constants::WINDOW_HEIGHT, 0);

	initShaders();
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
		//used for frame time measuring
		float startTicks = SDL_GetTicks();

		processInput();
		_time += 0.004; //need to setup time step

		_camera.update();

		drawGame();
		calcualteFPS();

		//print FPS every ten frames
		static int frameCounter = 0;
		++frameCounter;
		if (frameCounter == 10) {
			printf("FPS: %f\n", _fps);
			frameCounter = 0;
		}


		//Limit the FPS using SDL to delay processing
		float frameTicks = SDL_GetTicks() - startTicks; //this calculates how long it took to run the whole game loop
		static float targetFrameTicks = 1000.0f / constants::FPS_LIMIT;
		if (targetFrameTicks > frameTicks) {
			SDL_Delay(targetFrameTicks - frameTicks); //delay by difference between target and actual to sloooow thiiiiings dooown
		}

	}
}

void MainGame::processInput() {

	//check what the fack players have been doing
	SDL_Event event;

	static const float CAMERA_SPEED = 20.0f;
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
			switch (event.key.keysym.sym) { //testing camera movement
				case SDLK_UP:
					_camera.setPosition(_camera.getPosition() + glm::vec2(0.0, CAMERA_SPEED));
					break;
				case SDLK_DOWN:
					_camera.setPosition(_camera.getPosition() + glm::vec2(0.0, -CAMERA_SPEED));
					break;
				case SDLK_LEFT:
					_camera.setPosition(_camera.getPosition() + glm::vec2(-CAMERA_SPEED, 0.0));
					break;
				case SDLK_RIGHT:
					_camera.setPosition(_camera.getPosition() + glm::vec2(CAMERA_SPEED, 0.0));
					break;
				case SDLK_w:
					_camera.setScale(_camera.getScale() + SCALE_SPEED);
					break;
				case SDLK_q:
					_camera.setScale(_camera.getScale() + -SCALE_SPEED);
					break;
			}
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
	
	//set active texture 
	glActiveTexture(GL_TEXTURE0);
	GLint textureLocation = _colorShaderProgram.getUniformLocation("textureSampler");
	glUniform1i(textureLocation, 0);

	
	GLint timeLocation = _colorShaderProgram.getUniformLocation("time");
	glUniform1f(timeLocation, _time); //passing time to shader

	//Set the camera matrix
	GLint pLocation = _colorShaderProgram.getUniformLocation("P");
	glm::mat4 cameraMatrix = _camera.getCameraMatrix();
	glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(cameraMatrix[0][0]));


	for (int i = 0; i < _sprites.size(); ++i) {
		_sprites[i]->draw();
	}
	

	glBindTexture(GL_TEXTURE_2D, 0);
	_colorShaderProgram.unUse();

	_window.swapBuffer();
}

void MainGame::calcualteFPS() {
	static const int NUM_SAMPLES = 10; //how many frames we are going to average accross
	static float frameTimes[NUM_SAMPLES];
	static int currentFrame = 0;

	static float previousTicks = SDL_GetTicks();

	float currentTicks;
	currentTicks = SDL_GetTicks();

	_frameTime = currentTicks - previousTicks;
	frameTimes[currentFrame % NUM_SAMPLES] = _frameTime; //place into our circular buffer
	previousTicks = currentTicks;

	int count;
	if (++currentFrame < NUM_SAMPLES) { //if less than our sample size, only average what we have
		count = currentFrame;
	}
	else { //average accross NUM_SAMPLE frames
		count = NUM_SAMPLES;
	}

	float frameTimeAverage = 0;
	for (int i = 0; i < count; ++i) {
		frameTimeAverage += frameTimes[i]; //get total frame time
	}
	frameTimeAverage /= count; //calculate actual average

	if (frameTimeAverage > 0) {
		_fps = 1000.0f / frameTimeAverage;
	}
	else {
		_fps = 60.0f;
	}
}
