#pragma once

#include "Timing.h"

#include <SDL/SDL.h>

namespace OdinEngine {
	FPSLimiter::FPSLimiter() {}

	void FPSLimiter::init(float maxFPS) {
		setMaxFPS(maxFPS);
	}


	void FPSLimiter::setMaxFPS(float maxFPS) {
		_maxFPS = maxFPS;
	}

	//start fps limitting
	void FPSLimiter::beginFrame() {
		_startTicks = SDL_GetTicks();
	}

	//returns the fps
	float FPSLimiter::endFrame() {
		calculateFPS();

		//Limit the FPS using SDL to delay processing
		float frameTicks = SDL_GetTicks() - _startTicks; //this calculates how long it took to run the whole game loop
		static float targetFrameTicks = 1000.0f / _maxFPS;
		if (targetFrameTicks > frameTicks) {
			SDL_Delay(targetFrameTicks - frameTicks); //delay by difference between target and actual to sloooow thiiiiings dooown
		}

		return _fps;
	}

	void FPSLimiter::calculateFPS() {
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
}