// Andrew Meckling
#pragma once

#include <Odin/Scene.h>
#include <Odin/AudioEngine.h>
#include <Odin/Entity.hpp>
#include <Odin/TextureManager.hpp>

#include "Constants.h"

using odin::Entity;
using odin::EntityId;
using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::InputManager;
using odin::InputListener;
using odin::ComponentType;
using odin::AudioEngine;

class LevelScene
    : public odin::Scene
{
public:

    template< typename ValueType >
    using EntityMap = odin::BinarySearchMap< EntityId, ValueType >;

    EntityMap< Entity >             entities;

    EntityMap< GraphicalComponent > gfxComponents;

    b2ThreadPool                    b2thd;
    b2World                         b2world = { { 0.f, -9.81f }, &b2thd };
    EntityMap< PhysicalComponent >  fsxComponents;

    InputManager                    inputManager;
    EntityMap< InputListener >      listeners;


    std::string audioBankName;
    AudioEngine audioEngine;

    //SDL_Renderer* renderer;

    GLuint program;
    GLint uMatrix, uColor, uTexture, uFacingDirection,
        uCurrentFrame, uCurrentAnim, uMaxFrame, uMaxAnim;

    LevelScene( int width, int height, std::string audioBank = "" )
        : Scene( width, height )
        , audioBankName( std::move( audioBank ) )

        , program( load_shaders( "Shaders/vertexAnim.glsl", "Shaders/fragmentShader.glsl" ) )
        , uMatrix( glGetUniformLocation( program, "uMatrix" ) )
        , uColor( glGetUniformLocation( program, "uColor" ) )
        , uTexture( glGetUniformLocation( program, "uTexture" ) )
        , uFacingDirection( glGetUniformLocation( program, "uFacingDirection" ) )
        , uCurrentFrame( glGetUniformLocation( program, "uCurrentFrame" ) )
        , uCurrentAnim( glGetUniformLocation( program, "uCurrentAnim" ) )
        , uMaxFrame( glGetUniformLocation( program, "uMaxFrames" ) )
        , uMaxAnim( glGetUniformLocation( program, "uTotalAnim" ) )
    {
    }

    void init( unsigned ticks )
    {
        Scene::init( ticks );

        if ( audioBankName != "" )
        {
            audioEngine.loadBank( audioBankName + ".bank",
                                  FMOD_STUDIO_LOAD_BANK_NORMAL );
            audioEngine.loadBank( audioBankName + ".strings.bank",
                                  FMOD_STUDIO_LOAD_BANK_NORMAL );
        }
    }

    void exit( unsigned ticks )
    {
        Scene::exit( ticks );

        if ( audioBankName != "" )
        {
            audioEngine.unloadBank( audioBankName + ".bank" );
            audioEngine.unloadBank( audioBankName + ".strings.bank" );
        }
    }

    void update( unsigned ticks )
    {
        Scene::update( ticks );

        inputManager.pollEvents();
        for ( auto x : listeners )
        {
            x.value( inputManager, x.key );
        }

        //audioEngine.update();

        float timeStep = Scene::ticksDiff / 1000.f;
        b2world.Step( timeStep, 8, 3 );
        for ( auto x : fsxComponents )
        {
            Entity& ntt = entities[ x.key ];
            auto& fsx = x.value;

            ntt.position = fsx.position();

            if ( fsx.pBody->IsBullet() )
            {
                Vec2 vel = fsx.pBody->GetLinearVelocity();

                float angle = std::atan2( vel.y, vel.x );
                fsx.pBody->SetTransform( ntt.position, angle );
            }

            ntt.rotation = fsx.rotation();
        }
    }

    void draw()
    {
        using namespace glm;
        Scene::draw();

        float zoom = 1.0f / SCALE;
        float aspect = width / (float) height;
        const mat4 base = scale( {}, vec3( zoom, zoom * aspect, 1 ) );

        glUseProgram( program );
        for ( auto x : gfxComponents )
        {
            Entity& ntt = entities[ x.key ];
            auto& gfx = x.value;

            mat4 mtx = translate( base, vec3( ntt.position.glmvec2, 0 ) );
            mtx = rotate( mtx, ntt.rotation, vec3( 0, 0, 1 ) );

            gfx.incrementFrame();

            glUniform( uMatrix, mtx );
            glUniform( uColor, gfx.color );
            glUniform( uTexture, gfx.texture );
            glUniform( uFacingDirection, gfx.direction );
            glUniform( uCurrentFrame, gfx.currentFrame );
            glUniform( uCurrentAnim, gfx.animState );
            glUniform( uMaxFrame, gfx.maxFrames );
            glUniform( uMaxAnim, gfx.totalAnim );

            glBindVertexArray( gfx.vertexArray );
            glDrawArrays( GL_TRIANGLES, 0, gfx.count );
        }
    }

    void add( EntityId eid, GraphicalComponent gfx )
    {
        gfxComponents.add( eid, std::move( gfx ) );
    }

    void add( EntityId eid, PhysicalComponent fsx )
    {
        fsxComponents.add( eid, std::move( fsx ) );
    }

    void add( EntityId eid, InputListener lstn )
    {
        listeners.add( eid, std::move( lstn ) );
    }

    void player_input( const InputManager& mngr, EntityId eid );
};

struct EntityView
{
    EntityId    eid;
    LevelScene* pScene;

    EntityView( EntityId eid, LevelScene* game )
        : eid( eid )
        , pScene( game )
    {
    }

    void attach( GraphicalComponent gfx )
    {
        pScene->gfxComponents[ eid ] = std::move( gfx );
    }

    void attach( PhysicalComponent fsx )
    {
        pScene->fsxComponents[ eid ] = std::move( fsx );
    }

    void attach( InputListener lstn )
    {
        pScene->listeners[ eid ] = std::move( lstn );
    }

    void detach( ComponentType type )
    {
        switch ( type )
        {
        case ComponentType::Graphical:
            pScene->gfxComponents.remove( eid );
            break;
        case ComponentType::Physical:
            pScene->fsxComponents.remove( eid );
            break;
        case ComponentType::Input:
            pScene->listeners.remove( eid );
            break;
        }
    }

    GraphicalComponent* gfxComponent()
    {
        auto itr = pScene->gfxComponents.search( eid );
        return itr ? (GraphicalComponent*) itr : nullptr;
    }

    PhysicalComponent* fsxComponent()
    {
        auto itr = pScene->fsxComponents.search( eid );
        return itr ? (PhysicalComponent*) itr : nullptr;
    }

    InputListener* inputListener()
    {
        auto itr = pScene->listeners.search( eid );
        return itr ? (InputListener*) itr : nullptr;
    }
};

inline void LevelScene::player_input( const InputManager& mngr, EntityId eid )
{
    EntityView ntt = EntityView(eid, this);

    b2Body& body = *ntt.fsxComponent()->pBody;
    GraphicalComponent& gfx = *ntt.gfxComponent();

    Vec2 vel = body.GetLinearVelocity();
    float maxSpeed = 5.5f;
    float actionLeft = mngr.isKeyDown(SDLK_LEFT) ? 1.f : 0.f;
    float actionRight = mngr.isKeyDown(SDLK_RIGHT) ? 1.f : 0.f;

    //adjust facing direction
    if (actionLeft)
        gfx.direction = odin::LEFT;
    if (actionRight)
        gfx.direction = odin::RIGHT;

    //b2Fixture* pFixt = body.GetFixtureList();

    if (actionLeft == 0 && actionRight == 0)
    {
        //pFixt->SetFriction( 2 );
        vel.x = tween<float>(vel.x, 0, 12 * (1 / 60.0f));
        gfx.switchAnimState(0); //idle state
    }
    else
    {
        //pFixt->SetFriction( 0 );
        vel.x -= actionLeft * (20 + 1) * (1 / 60.0f);
        vel.x += actionRight * (20 + 1) * (1 / 60.0f);
        vel.x = glm::clamp(vel.x, -maxSpeed, +maxSpeed);
        gfx.switchAnimState(1); //running
    }

    if (mngr.wasKeyPressed(SDLK_UP)) {
        vel.y = 11;
    }

    if (mngr.wasKeyReleased(SDLK_UP) && vel.y > 0) {
        vel.y *= 0.6f;
    }

    if (mngr.gamepads.wasButtonPressed(0, SDL_CONTROLLER_BUTTON_A))
        vel.y = 11;

    if (mngr.gamepads.wasButtonReleased(0, SDL_CONTROLLER_BUTTON_A) && vel.y > 0)
        vel.y *= 0.6f;

    //for testing audio
    if (mngr.wasKeyPressed(SDLK_SPACE))
        audioEngine.playEvent("event:/Desperado/Shoot"); //simulate audio shoot
    if (mngr.wasKeyPressed(SDLK_1))
        audioEngine.setEventParameter("event:/Music/EnergeticTheme", "Energy", 0.0); //low energy test
    if (mngr.wasKeyPressed(SDLK_2))
        audioEngine.setEventParameter("event:/Music/EnergeticTheme", "Energy", 1.0); //high energy test

    body.SetLinearVelocity(vel);
}