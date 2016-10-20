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

class Game
{
public:
	SceneManager sceneManager;
	AudioEngine audioEngine;

    int _width;
    int _height;

	bool running = false;

	const double tgtFrameTime = 1 / 60.0;
	const int tgtFrameTime_ms = int( tgtFrameTime * 1000 );
	int tFrameStart = 0;

	Game( int width, int height, SDL_Window* window )
		: _width(width)
		, _height(height)
		, sceneManager()
	{
		//audio engine setup
		//this must come before sceneManager, as scenes rely on the engine
		audioEngine.init();

		//set up renderer for drawing through SDL
		//renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        sceneManager.pushScene( new TestScene( _width / 4, _height / 4, SCALE ) );
	}

    void tick()
    {
        unsigned frameStart = SDL_GetTicks();

        sceneManager.update( frameStart );
        audioEngine.update();

        sceneManager.draw();

        if ( Scene* top = sceneManager.topScene() )
        {
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            glViewport( 0, 0, _width, _height );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

            glBlitNamedFramebuffer(
                sceneManager.topScene()->framebuffer.frame, 0,
                0, 0, top->width, top->height,
                0, 0, _width, _height,
                GL_COLOR_BUFFER_BIT,// | GL_DEPTH_BUFFER_BIT,
                GL_NEAREST );
        }

        // Cap framerate at 60fps
        int frameEnd = SDL_GetTicks();
        int frameTime_ms = (frameEnd - frameStart);

        if ( frameTime_ms <= tgtFrameTime_ms )
            SDL_Delay( tgtFrameTime_ms - frameTime_ms );
        else
            printf( "Frame %ims slow\n", frameTime_ms - tgtFrameTime_ms );
    }
 
};
