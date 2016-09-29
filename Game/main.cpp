
#ifndef _DEBUG
#define SDL_MAIN_HANDLED
#endif

#include <SDL/SDL.h>
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "Game.h"
#include "glhelp.h"
#include <lodepng.h>

#include <thread>
#include <chrono>

template< typename Array >
GLuint load_texture( int index, int width, int height, const Array& data )
{
    GLuint tex;
    glGenTextures( 1, &tex );

    glActiveTexture( GL_TEXTURE0 + index );
    glBindTexture( GL_TEXTURE_2D, tex );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, (void*) std::data( data ) );

    glGenerateMipmap( GL_TEXTURE_2D );

    return tex;
}

GLuint load_texture( int index, const char* filename )
{
    std::vector< GLubyte > image;
    unsigned width, height;

    if ( unsigned error = lodepng::decode( image, width, height, filename ) )
        printf( "Error %u: %s\n", error, lodepng_error_text( error ) );

    return load_texture( index, width, height, image );
}

SDL_Window* create_window( const char* title )
{
    //Create OpenGL window
    //SDL_sdlWindowPOS_CENTERED creates window in the center position using given width/height
    SDL_Window* _sdlWindow = SDL_CreateWindow( title, SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL );
    //simple error checking
    if ( _sdlWindow == nullptr ) {
        printf( "SDL_Window could not be created.\n" );
        SDL_Quit();
    }

    SDL_GLContext glContext = SDL_GL_CreateContext( _sdlWindow );
    if ( glContext == nullptr ) {
        printf( "SDL_GLContext could not be created.\n" );
        SDL_Quit();
    }

    if ( GLenum err = glewInit() ) { // != GLEW_OK
        fprintf( stderr, "Error: %s\n", glewGetErrorString( err ) );
        SDL_Quit();
    }

    SDL_GL_SetSwapInterval( 0 );
    printf( "OpenGL %s\n\n", glGetString( GL_VERSION ) );

    return _sdlWindow;
}

void setup_game( Game& game );
void process_events( Game& game );

int main( int argc, char** argv )
{
    SDL_Window* sdl_window = create_window( "One Horse Town" );

    Game game;
    setup_game( game );

    // main loop
    for ( game.running = true; game.running; SDL_GL_SwapWindow( sdl_window ) )
    {
        const double tgtFrameTime = 1 / 60.0;
        const int tgtFrameTime_ms = tgtFrameTime * 1000;

        int tFrameStart = SDL_GetTicks();

        process_events( game );

        game.b2world.Step( tgtFrameTime, 50, 50 );
        for ( auto itr = game.fsxComponents.begin();
              itr != game.fsxComponents.end(); ++itr )
        {
            game.update( *itr, itr.key() );
        }

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        for ( auto itr = game.gfxComponents.begin();
              itr != game.gfxComponents.end(); ++itr )
        {
            game.draw( *itr, itr.key() );
        }

        // Cap framerate at 60fps
        int tFrameEnd = SDL_GetTicks();
        int frameTime_ms = (tFrameEnd - tFrameStart);

        if ( frameTime_ms <= tgtFrameTime_ms )
            SDL_Delay( tgtFrameTime_ms - frameTime_ms );
        else
            printf( "Frame %ims slow\n", frameTime_ms - tgtFrameTime_ms );

        //printf( "ft: %ims\n", frameTime_ms );
    }
    // exit main loop

    SDL_DestroyWindow( sdl_window );
    SDL_Quit();
    return 0;
}

void process_events( Game& game )
{
    SDL_Event event;
    //while the players have actually mashed some buttons or keys
    while ( SDL_PollEvent( &event ) )
    {
        switch ( event.type )
        {
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
        case SDL_KEYDOWN: {
            //key press
            //_inputManager.keyDown(event.key.keysym.sym);
            auto playerBody = game.fsxComponents[ "player" ].pBody;
            Vec2 vel = playerBody->GetLinearVelocity();
            switch ( event.key.keysym.sym )
            {
            case SDLK_LEFT:
                playerBody->SetLinearVelocity( vel + Vec2{ -1, 0 } );
                break;
            case SDLK_RIGHT:
                playerBody->SetLinearVelocity( vel + Vec2{ 1, 0 } );
                break;
            case SDLK_UP:
                playerBody->SetLinearVelocity( vel + Vec2{ 0, 10 } );
                break;
            case SDLK_DOWN:
                playerBody->SetLinearVelocity( vel + Vec2{ 0, -10 } );
                break;
            }
            //game.fireBullet( {-20, 0}, {100, 0} );
            break;
        }
        case SDL_KEYUP:
            //key press
            //_inputManager.keyUp(event.key.keysym.sym);
            break;
        case SDL_QUIT:
            game.running = false;
            break;
        }
    }
}

void setup_game( Game& game )
{
    game.gfxComponents.add( "player", GraphicalComponent::makeRect( 0.5, 1.5 ) );

    b2BodyDef playerDef;
    playerDef.position = { -7, -4 };
    playerDef.fixedRotation = true;
    playerDef.type = b2_dynamicBody;

    game.fsxComponents.add( "player", PhysicalComponent::makeRect( 0.5, 1.5, game.b2world, playerDef ) );

    game.addEqTri( {"tri", 0}, 2, {5, -3}, 0, {1, 0, 0}, b2_kinematicBody );
    auto whttri = game.addEqTri( {"tri", 1}, 2, {0, 0}, 0, {1, 1, 1}, b2_dynamicBody );
    game.addEqTri( {"tri", 2}, 2, {0.5, 2.5}, 0, {0, 0, 1}, b2_dynamicBody );
    game.addEqTri( {"tri", 3}, 2, {7, -1}, 0, {0, 1, 0}, b2_kinematicBody );

    game.fsxComponents[ {"tri", 0} ].pBody->SetAngularVelocity( -1 );
    game.fsxComponents[ {"tri", 3} ].pBody->SetAngularVelocity( 2 );

    b2RevoluteJointDef rjd;
    b2BodyDef bodyDef;
    auto ground = game.b2world.CreateBody( &bodyDef );
    rjd.Initialize( ground, game.fsxComponents[ {"tri", 1} ].pBody, { 0, 0 } );
    game.b2world.CreateJoint( &rjd );

    //             eid    dimens   pos   rot   color       body_type
    game.addRect( "rect", {1, 2}, {0, 7}, 0, {1, 1, 0}, b2_dynamicBody );
    game.addRect( {"box", 0}, {1, 1}, {-.5, 4}, 0, {1, 0, 1}, b2_dynamicBody );
    auto pnkcrate = game.addRect( {"box", 1}, {1, 1}, {-.5, 5}, 0, {1, 0, 1}, b2_dynamicBody );
    game.addRect( {"box", 2}, {1, 1}, {-.5, 6}, 0, {1, 0, 1}, b2_dynamicBody );
    game.addRect( {"box", 3}, {1, 1}, {-.5, 7}, 0, {1, 0, 1}, b2_dynamicBody );
    game.addRect( {"box", 4}, {1, 1}, {-.5, 8}, 0, {1, 0, 1}, b2_dynamicBody );
    auto blucrate = game.addRect( {"box", 5}, {1, 1}, {.1f, 90}, 0, {0, 1, 1}, b2_dynamicBody );
    game.addRightTri( {"rtri", 0}, {-2, 2}, {0, -3}, 0, {0, 1, 1}, b2_dynamicBody );
    auto ylwramp = game.addRightTri( {"rtri", 1}, {3, 1}, {-2, -3}, 0, {1, 1, 0} );

    game.fireBullet( {-170, 5.5f}, {100, 0} );

    //GLuint nul = load_texture( "null.png", 0 );
    GLuint nul = load_texture< GLubyte[4] >( 0, 1, 1, { 0xFF, 0xFF, 0xFF, 0xFF } );
    GLuint tex = load_texture( 1, "crate.png" );
    GLuint tex2 = load_texture( 2, "crate2.png" );

    blucrate.gfxComponent()->texture = 2;
    pnkcrate.gfxComponent()->texture = 1;
    ylwramp.gfxComponent()->texture = 1;
    whttri.gfxComponent()->texture = 1;

    b2BodyDef floorDef;
    b2EdgeShape floorShape;
    floorShape.Set( {-WIDTH / SCALE, -HEIGHT / SCALE},
    {WIDTH / SCALE, -HEIGHT / SCALE} );

    game.fsxComponents[ "floor" ] = game.b2world.CreateBody( &floorDef );
    game.fsxComponents[ "floor" ].pBody->CreateFixture( &floorShape, 1 );

    floorShape.Set( {WIDTH / SCALE, +HEIGHT / SCALE},
    {WIDTH / SCALE, -HEIGHT / SCALE} );

    game.fsxComponents[ "wall" ] = game.b2world.CreateBody( &floorDef );
    game.fsxComponents[ "wall" ].pBody->CreateFixture( &floorShape, 1 );
}

