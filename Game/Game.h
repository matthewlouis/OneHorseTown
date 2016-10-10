// Andrew Meckling
#pragma once

#include <Odin/Entity.hpp>
#include "Constants.h"
#include <Odin/SceneManager.hpp>
#include "TestScene.hpp"

using odin::Entity;
using odin::EntityId;
using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::InputManager;
using odin::InputListener;
using odin::ComponentType;
using odin::SceneManager;

enum Scenes {
	TEST
};

class Game
{
public:
	SceneManager sceneManager;

	GLint                           uMatrix, uColor, uTexture;
	GLuint                          program;

    int _width;
    int _height;

	bool running = false;

	const double tgtFrameTime = 1 / 60.0;
	const int tgtFrameTime_ms = tgtFrameTime * 1000;
	int tFrameStart = 0;

	Game(int width, int height)
		: program(loadShaders("vertexShader.glsl", "fragmentShader.glsl"))
		, uMatrix(glGetUniformLocation(program, "uMatrix"))
		, uColor(glGetUniformLocation(program, "uColor"))
		, uTexture(glGetUniformLocation(program, "uTexture"))
		, _width(width)
		, _height(height)
		, sceneManager()
    {
		sceneManager.addScene(TEST, new TestScene(_width, _height, SCALE));
		sceneManager.changeScene(TEST);
    }

    
	void handleInput();
    void update();
	void draw();
   
};

void Game::handleInput() {
	tFrameStart = SDL_GetTicks();

	sceneManager.handleInput();
}

void Game::update() {
	sceneManager.update(tgtFrameTime);
}

void Game::draw() {

	
	glViewport(0, 0, LOW_WIDTH, LOW_HEIGHT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	float zoom = 1.0 / SCALE;
	float aspect = _width / (float)_height;

	sceneManager.draw(zoom, aspect, uMatrix, uColor, uTexture, program);

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


