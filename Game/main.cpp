
#include <Odin/Odin.h>

#include <glm/glm.hpp>

#include "Game.h"

//#include "Allocators.hpp"
//#include "ContextAllocator.hpp"

#include <Odin/SDLAudio.h>

SDL_Window* create_window( const char* title, int width, int height );

int main( int argc, char** argv )
{
    srand((unsigned)time(NULL));

    SDL_Window* sdl_window = create_window( "One Horse Town", WIDTH, HEIGHT );

    Game game( WIDTH, HEIGHT, sdl_window );

    // main loop
    for ( game.running = true; game.running; SDL_GL_SwapWindow( sdl_window ) )
    {
        game.tick();
    }
    // exit main loop

    SDL_DestroyWindow( sdl_window );
    SDL_Quit();
    return 0;
}

SDL_Window* create_window( const char* title, int width, int height )
{
	//previously we were not initting all of the subsystems.
	//best thing to do here is init everything
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL_Init failed: %s.\n", SDL_GetError());
		SDL_Quit();
	}

	initAudio(); //must call first for SDL audio playback

	//Request minimum version for compatibility.
	if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3))
		printf("SDL_GL_SetAttribute failed: %s.\n", SDL_GetError());
	if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3))
		printf("SDL_GL_SetAttribute failed: %s.\n", SDL_GetError());
	if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE))
		printf("SDL_GL_SetAttribute failed: %s.\n", SDL_GetError());

    //Create OpenGL window
    //SDL_sdlWindowPOS_CENTERED creates window in the center position using given width/height
    SDL_Window* _sdlWindow = SDL_CreateWindow( title, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL );
    //simple error checking
    if ( _sdlWindow == nullptr )
    {
        printf( "SDL_Window could not be created.\n" );
        SDL_Quit();
    }

    SDL_GLContext glContext = SDL_GL_CreateContext( _sdlWindow );
    if ( glContext == nullptr )
    {
        printf( "SDL_GLContext could not be created.\n" );
        SDL_Quit();
    }

    if ( GLenum err = glewInit() )
    {   // != GLEW_OK
        fprintf( stderr, "Error: %s\n", glewGetErrorString( err ) );
        SDL_Quit();
    }

    SDL_GL_SetSwapInterval( 0 );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    printf( "OpenGL %s\n\n", glGetString( GL_VERSION ) );


	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    return _sdlWindow;
}


