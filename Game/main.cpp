
#include <Odin/Odin.h>

#include <glm/glm.hpp>

#include "Game.h"

SDL_Window* create_window( const char* title, int width, int height )
{
	//previously we were not initting all of the subsystems.
	//best thing to do here is init everything
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL_Init failed: %s.\n", SDL_GetError());
		SDL_Quit();
	}

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

    return _sdlWindow;
}

// void setup_game( Game& game );

int main( int argc, char** argv )
{
    SDL_Window* sdl_window = create_window( "One Horse Town", WIDTH, HEIGHT );

    auto framebuffer = odin::make_framebuffer( LOW_WIDTH, LOW_HEIGHT,
                                               odin::Framebuffer::COLOR );

    GLuint frameBuffer = 0;
    glGenFramebuffers( 1, &frameBuffer );
    glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );


    // The texture we're going to render to
    GLuint renderedTexture;
    glGenTextures( 1, &renderedTexture );

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture( GL_TEXTURE_2D, renderedTexture );

    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, LOW_WIDTH, LOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );

    // Poor filtering. Needed !
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    // The depth buffer
    //GLuint depthrenderbuffer;
    //glGenRenderbuffers( 1, &depthrenderbuffer );
    //glBindRenderbuffer( GL_RENDERBUFFER, depthrenderbuffer );
    //glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, LOW_WIDTH, LOW_HEIGHT );
    //glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer );

    // Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0 );

    // Set the list of draw buffers.
    //GLenum DrawBuffers[ 1 ] = { GL_COLOR_ATTACHMENT0 };
    //glDrawBuffers( 1, DrawBuffers ); // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
        return false;

    Game game( WIDTH, HEIGHT, sdl_window );
    // setup_game( game );

    // main loop
    for ( game.running = true; game.running; SDL_GL_SwapWindow( sdl_window ) )
    {

		game.handleInput();
		game.update();

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		game.draw();

		glBlitNamedFramebuffer(frameBuffer, 0,
			0, 0, LOW_WIDTH, LOW_HEIGHT,
			0, 0, WIDTH, HEIGHT,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
			GL_NEAREST);
        

        //printf( "ft: %ims\n", frameTime_ms );
    }
    // exit main loop

    SDL_DestroyWindow( sdl_window );
    SDL_Quit();
    return 0;
}


