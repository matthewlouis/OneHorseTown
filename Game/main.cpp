
#include <Odin/Odin.h>

#include <glm/glm.hpp>

#include "Game.h"

SDL_Window* create_window( const char* title, int width, int height )
{
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

    if ( SDL_Init( SDL_INIT_GAMECONTROLLER ) != 0 )
    {
        printf( "SDL_Init failed: %s.\n", SDL_GetError() );
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

void setup_game( Game& game );

const int LOW_WIDTH = WIDTH / PIXEL_SIZE;
const int LOW_HEIGHT = HEIGHT / PIXEL_SIZE;

int main( int argc, char** argv )
{
    SDL_Window* sdl_window = create_window( "One Horse Town", WIDTH, HEIGHT );

    auto framebuffer = odin::make_framebuffer( LOW_WIDTH, LOW_HEIGHT,
                                               odin::Framebuffer::COLOR );

    Game game( WIDTH, HEIGHT );
    setup_game( game );

    // main loop
    for ( game.running = true; game.running; SDL_GL_SwapWindow( sdl_window ) )
    {
        const double tgtFrameTime = 1 / 60.0;
        const int tgtFrameTime_ms = tgtFrameTime * 1000;

        int tFrameStart = SDL_GetTicks();

        game.inputManager.pollEvents( [&]() { game.running = false; } );
        for ( auto itr = game.listeners.begin();
        itr != game.listeners.end(); ++itr )
        {
            game.trigger( *itr, itr.key() );
        }

        game.b2world.Step( tgtFrameTime, 50, 50 );
        for ( auto itr = game.fsxComponents.begin();
        itr != game.fsxComponents.end(); ++itr )
        {
            game.update( *itr, itr.key() );
        }

        glBindFramebuffer( GL_FRAMEBUFFER, framebuffer.frame );
        glViewport( 0, 0, LOW_WIDTH, LOW_HEIGHT );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        for ( auto itr = game.gfxComponents.begin();
        itr != game.gfxComponents.end(); ++itr )
        {
            game.draw( *itr, itr.key() );
        }

        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glViewport( 0, 0, WIDTH, HEIGHT );
        
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glBlitNamedFramebuffer( framebuffer.frame, 0,
                                0, 0, LOW_WIDTH, LOW_HEIGHT,
                                0, 0, WIDTH, HEIGHT,
                                GL_COLOR_BUFFER_BIT,// | GL_DEPTH_BUFFER_BIT,
                                GL_NEAREST );

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

void player_input( const InputManager& mngr, EntityView ntt )
{
    b2Body& body = *ntt.fsxComponent()->pBody;

    Vec2 vel = body.GetLinearVelocity();
    float maxSpeed = 5.5f;
    float actionLeft = mngr.isKeyDown( SDLK_LEFT ) ? 1 : 0;
    float actionRight = mngr.isKeyDown( SDLK_RIGHT ) ? 1 : 0;

    //b2Fixture* pFixt = body.GetFixtureList();

    if ( actionLeft == 0 && actionRight == 0 )
    {
        //pFixt->SetFriction( 2 );
        vel.x = tween<float>( vel.x, 0, 12 * (1/60.0) );
    }
    else
    {
        //pFixt->SetFriction( 0 );
        vel.x -= actionLeft * (20 + 1) * (1/60.0);
        vel.x += actionRight * (20 + 1) * (1/60.0);
        vel.x = glm::clamp( vel.x, -maxSpeed, +maxSpeed );
    }

    if ( mngr.wasKeyPressed( SDLK_UP ) )
        vel.y = 11;

    if ( mngr.wasKeyReleased( SDLK_UP ) && vel.y > 0 )
        vel.y *= 0.6f;

    if ( mngr.gamepads.wasButtonPressed( 0, SDL_CONTROLLER_BUTTON_A ) )
        vel.y = 11;

    if ( mngr.gamepads.wasButtonReleased( 0, SDL_CONTROLLER_BUTTON_A ) && vel.y > 0 )
        vel.y *= 0.6f;

    body.SetLinearVelocity( vel );
}


void setup_game( Game& game )
{
    auto background = game.gfxComponents.add(
        EntityId( 0 ), GraphicalComponent::makeRect( game._width / SCALE, game._height / SCALE ) );
    background->texture = 4;

    auto pGfx = game.gfxComponents.add( "player", GraphicalComponent::makeRect( 1, 2.33 ) );
    pGfx->texture = 3;

    b2BodyDef playerDef;
    playerDef.position = { -7, -4 };
    playerDef.fixedRotation = true;
    playerDef.type = b2_dynamicBody;
    playerDef.gravityScale = 2;

    auto pFsx = game.fsxComponents.add( "player", PhysicalComponent::makeRect( 1, 2.33, game.b2world, playerDef ) );

    game.listeners.add( "player", [&game]( const InputManager& inmn, EntityId eid ) {
        return player_input( inmn, EntityView( eid, &game ) );
    } );

    game.addEqTri( { "tri", 0 }, 2, { 5, -3 }, 0, { 1, 0, 0 }, b2_kinematicBody );
    auto whttri = game.addEqTri( { "tri", 1 }, 2, { 0, 0 }, 0, { 1, 1, 1 }, b2_dynamicBody );
    game.addEqTri( { "tri", 2 }, 2, { 0.5, 2.5 }, 0, { 0, 0, 1 }, b2_dynamicBody );
    game.addEqTri( { "tri", 3 }, 2, { 7, -1 }, 0, { 0, 1, 0 }, b2_kinematicBody );

    game.fsxComponents[ {"tri", 0} ]->SetAngularVelocity( -1 );
    game.fsxComponents[ {"tri", 3} ]->SetAngularVelocity( 2 );

    b2RevoluteJointDef rjd;
    b2BodyDef bodyDef;
    auto ground = game.b2world.CreateBody( &bodyDef );
    rjd.Initialize( ground, game.fsxComponents[ {"tri", 1} ].pBody, { 0, 0 } );
    game.b2world.CreateJoint( &rjd );

    //             eid    dimens   pos   rot   color       body_type
    game.addRect( "rect", { 1, 2 }, { 0, 7 }, 0, { 1, 1, 0 }, b2_dynamicBody );
    game.addRect( { "box", 0 }, { 1, 1 }, { -.5, 4 }, 0, { 1, 0, 1 }, b2_dynamicBody );
    auto pnkcrate = game.addRect( { "box", 1 }, { 1, 1 }, { -.5, 5 }, 0, { 1, 0, 1 }, b2_dynamicBody );
    game.addRect( { "box", 2 }, { 1, 1 }, { -.5, 6 }, 0, { 1, 0, 1 }, b2_dynamicBody );
    game.addRect( { "box", 3 }, { 1, 1 }, { -.5, 7 }, 0, { 1, 0, 1 }, b2_dynamicBody );
    game.addRect( { "box", 4 }, { 1, 1 }, { -.5, 8 }, 0, { 1, 0, 1 }, b2_dynamicBody );
    auto blucrate = game.addRect( { "box", 5 }, { 1, 1 }, { .1f, 90 }, 0, { 0, 1, 1 }, b2_dynamicBody );
    game.addRightTri( { "rtri", 0 }, { -2, 2 }, { 0, -3 }, 0, { 0, 1, 1 }, b2_dynamicBody );
    auto ylwramp = game.addRightTri( { "rtri", 1 }, { 3, 1 }, { -2, -3 }, 0, { 1, 1, 0 } );

    game.fireBullet( { -170, 5.5f }, { 100, 0 } );

    //GLuint nul = load_texture( "null.png", 0 );
    GLuint nul = odin::load_texture< GLubyte[ 4 ] >( 0, 1, 1, { 0xFF, 0xFF, 0xFF, 0xFF } );
    GLuint tex = odin::load_texture( 1, "Textures/crate.png" );
    GLuint tex2 = odin::load_texture( 2, "Textures/crate2.png" );
    GLuint tex3 = odin::load_texture( 3, "Textures/pixel_cowboy.png" );
    GLuint tex4 = odin::load_texture( 4, "Textures/background.png" );

    blucrate.gfxComponent()->texture = 2;
    pnkcrate.gfxComponent()->texture = 1;
    ylwramp.gfxComponent()->texture = 1;
    whttri.gfxComponent()->texture = 1;

    b2BodyDef floorDef;
    b2EdgeShape floorShape;
    floorShape.Set( { -10, -8 }, { 10, -8 } );

    game.fsxComponents[ "floor" ] = game.b2world.CreateBody( &floorDef );
    game.fsxComponents[ "floor" ]->CreateFixture( &floorShape, 1 )
        ->SetFriction( odin::PhysicalComponent::DEFAULT_FRICTION );

    floorShape.Set( { 10, +8 }, { 10, -8 } );

    game.fsxComponents[ "wall" ] = game.b2world.CreateBody( &floorDef );
    game.fsxComponents[ "wall" ]->CreateFixture( &floorShape, 1 )
        ->SetFriction( odin::PhysicalComponent::DEFAULT_FRICTION );
}

