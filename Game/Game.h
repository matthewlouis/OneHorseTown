// Andrew Meckling
#pragma once

#include <Odin/Entity.hpp>
#include "Constants.h"
#include <Odin/SceneManager.hpp>
#include "TestScene.hpp"
#include <Odin\AudioEngine.h>

using odin::Entity;
using odin::EntityId;
using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::InputManager;
using odin::InputListener;
using odin::ComponentType;
using odin::SceneManager;
using odin::AudioEngine;

enum Scenes {
	TEST
};

class Game
{
public:
	SceneManager sceneManager;
	AudioEngine audioEngine;

	SDL_Renderer* renderer;
	
	GLuint                          program;
    int _width;
    int _height;

	bool running = false;

	const double tgtFrameTime = 1 / 60.0;
	const int tgtFrameTime_ms = int( tgtFrameTime * 1000 );
	int tFrameStart = 0;

	Game(int width, int height, SDL_Window *window)
		: program(load_shaders("Shaders/vertexAnim.glsl", "Shaders/fragmentShader.glsl"))
		, _width(width)
		, _height(height)
		, sceneManager()
	{
		//audio engine setup
		//this must come before sceneManager, as scenes rely on the engine
		audioEngine.init();

		//set up renderer for drawing through SDL
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

		sceneManager.addScene(TEST, new TestScene(_width, _height, SCALE, program, renderer));
		sceneManager.changeScene(TEST);
	}

		void handleInput();
		void update();
		void draw();  
};

void Game::handleInput() {

	sceneManager.handleInput();
}

void Game::update() {
	sceneManager.update( (float) tgtFrameTime );

	audioEngine.update(); 
}

void Game::draw() {

	tFrameStart = SDL_GetTicks();

	glViewport(0, 0, LOW_WIDTH, LOW_HEIGHT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	float zoom = 1.0f / SCALE;
	float aspect = _width / (float)_height;

	sceneManager.draw(zoom, aspect, program);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WIDTH, HEIGHT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	


	// Cap framerate at 60fps
	int tFrameEnd = SDL_GetTicks();
	int frameTime_ms = (tFrameEnd - tFrameStart);

	if (frameTime_ms <= tgtFrameTime_ms)
		SDL_Delay(tgtFrameTime_ms - frameTime_ms);
	else
		printf("Frame %ims slow\n", frameTime_ms - tgtFrameTime_ms);
}


