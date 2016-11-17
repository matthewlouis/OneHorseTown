// Andrew Meckling
#pragma once

#include <Odin/Entity.hpp>
#include "Constants.h"
#include <Odin/SceneManager.hpp>
#include "TestScene.hpp"
#include "TitleScene.hpp"
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

    InputManager inputManager;
	SceneManager sceneManager;
	AudioEngine  audioEngine;

    int _width;
    int _height;

	bool running = false;

    static constexpr double TGT_FRAME_TIME_S = 1 / 60.0;
    static constexpr unsigned TGT_FRAME_TIME_MS = unsigned( TGT_FRAME_TIME_S * 1000 );

	Game( int width, int height, SDL_Window* window )
		: _width( width )
		, _height( height )
	{
		//audio engine setup
		//this must come before sceneManager, as scenes rely on the engine
		audioEngine.init();

        //auto scene = new TestScene( _width, _height, SCALE * PIXEL_SIZE );

		//note TestScene now takes number of players
		auto title = new TitleScene( _width / PIXEL_SIZE, _height / PIXEL_SIZE, "Audio/Banks/MasterBank");
		title->pInputManager = &inputManager;
		title->pAudioEngine = &audioEngine;
        title->pSceneManager = &sceneManager;

		sceneManager.pushScene( title );
        //sceneManager.pushScene( level );
	}

    void tick()
    {
        unsigned frameStart = SDL_GetTicks();

        inputManager.pollEvents();

        sceneManager.update( frameStart );
        sceneManager.render();

        audioEngine.update();


        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glViewport( 0, 0, _width, _height );
        glClear( GL_COLOR_BUFFER_BIT );// | GL_DEPTH_BUFFER_BIT );

        if ( Scene* top = sceneManager.topScene() )
        {
            glBindFramebuffer( GL_READ_FRAMEBUFFER, top->framebuffer.frame );
            glReadBuffer( GL_COLOR_ATTACHMENT0 );

            glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
            GLenum drawbuf = GL_COLOR_ATTACHMENT0;
            glDrawBuffers( 1, &drawbuf );

            glBlitFramebuffer(
            //glBlitNamedFramebuffer(
            //    top->framebuffer.frame, 0,
                0, 0, top->width, top->height,
                0, 0, _width, _height,
                GL_COLOR_BUFFER_BIT,// | GL_DEPTH_BUFFER_BIT,
                GL_NEAREST );
        }

        // Cap framerate at 60fps
        unsigned frameEnd = SDL_GetTicks();
        unsigned frameTime_ms = frameEnd - frameStart;

        if ( frameTime_ms <= TGT_FRAME_TIME_MS )
            SDL_Delay( TGT_FRAME_TIME_MS - frameTime_ms );
        else
            printf( "Frame %ims slow\n", frameTime_ms - TGT_FRAME_TIME_MS );
    }
 
};
