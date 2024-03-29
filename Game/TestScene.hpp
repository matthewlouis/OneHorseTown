#pragma once

#include <Odin/SceneManager.hpp>
#include <Odin/TextureManager.hpp>

#include "EntityFactory.h"
#include "Scenes.hpp"

#include <functional>
#include <future>

#include "OpenCLKernel.h"

using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::InputManager;
using odin::Entity;
using odin::EntityId;
using odin::Scene;

struct Particle
{
    float     lifetime;
    cl_float4 color = { 1, 1, 1, 1 };
    cl_float2 position = { 0, 0 };
    cl_float2 velocity = { 0, 0 };
    
    Particle() = default;

    Particle( float lifetime, glm::vec4 color,
              glm::vec2 pos, glm::vec2 vel )
        : lifetime( lifetime )
        , color( { color.x, color.y, color.z, color.w } )
        , position( { pos.x, pos.y } )
        , velocity( { vel.x, vel.y } )
    {
    }

    bool isExpired() const
    {
        return lifetime <= 0;
    }

    static void default_update( Particle& p, float step )
    {
        p.lifetime -= step;
        p.position.x += p.velocity.x * step;
        p.position.y += p.velocity.y * step;
    }
};

class ParticleEmitter
{
public:

    bool active = true;
    glm::vec2 position;

    float spawnRate;

    float velocityMagnitude;
    float velocityMagnitudeVariance;
    float velocityAngle;
    float velocityAngleVariance;

    float lifetime;
    float lifetimeVariance;

    glm::vec4 color = { 1, 1, 1, 1 };
    glm::vec4 colorVariance = { 0, 0, 0, 0 };

    std::vector< Particle > particles;

    std::function< void( Particle&, float ) > fnUpdate;

    ParticleEmitter( glm::vec2 position )
        : position( position )
    {
        particles.reserve( 50 );
    }

    ParticleEmitter( const ParticleEmitter& copy ) = delete;
    ParticleEmitter& operator =( const ParticleEmitter& copy ) = delete;

    ParticleEmitter( ParticleEmitter&& move )
        : active( move.active )
        , position( move.position )
        , spawnRate( move.spawnRate )
        , velocityMagnitude( move.velocityMagnitude )
        , velocityMagnitudeVariance( move.velocityMagnitudeVariance )
        , velocityAngle( move.velocityAngle )
        , velocityAngleVariance( move.velocityAngleVariance )
        , lifetime( move.lifetime )
        , lifetimeVariance( move.lifetimeVariance )
        , color( move.color )
        , colorVariance( move.colorVariance )
        , particles( std::move( move.particles ) )
        , fnUpdate( std::move( move.fnUpdate ) )
    {
    }

    ParticleEmitter& operator =( ParticleEmitter&& move )
    {
        active = move.active;
        position = move.position;
        spawnRate = move.spawnRate;
        velocityMagnitude = move.velocityMagnitude;
        velocityMagnitudeVariance = move.velocityMagnitudeVariance;
        velocityAngle = move.velocityAngle;
        velocityAngleVariance = move.velocityAngleVariance;
        lifetime = move.lifetime;
        lifetimeVariance = move.lifetimeVariance;
        color = move.color;
        colorVariance = move.colorVariance;
        particles = std::move( move.particles );
        fnUpdate = std::move( move.fnUpdate );
        return *this;
    }

    ~ParticleEmitter() = default;

    Particle* emitt( int n = 1 )
    {
        if ( n < 1 )
            return nullptr;

        particles.reserve( particles.size() + n );
        size_t first = particles.size();

        while ( n-- > 0 )
        {
            float length = apply_variance( velocityMagnitude, velocityMagnitudeVariance );
            float angle = apply_variance( velocityAngle, velocityAngleVariance );

            glm::vec2 vel = {
                length * glm::cos( angle ),
                length * glm::sin( angle )
            };

            particles.push_back( Particle(
            apply_variance( lifetime, lifetimeVariance ),
            apply_variance( color, colorVariance ),
                position, vel ) );
        }

        return &particles[ first ];
    }

    void spawn( float timeStep )
    {
        float numToEmitt = std::rand() / float( RAND_MAX );
        numToEmitt *= spawnRate * timeStep * 10;

        emitt( (int) glm::round( numToEmitt ) );
    }

    void update( float timeStep )
    {
        for ( Particle& p : particles )
        {
            p.velocity.y += -100 * timeStep;
            Particle::default_update( p, timeStep );
        }

        auto split = std::remove_if( particles.begin(), particles.end(),
            []( Particle& p ) {
                return p.isExpired();
            } );

        if ( fnUpdate )
            std::for_each( particles.begin(), split,
                [this, timeStep]( Particle& p ) {
                    fnUpdate( p, timeStep );
                } );

        particles.erase( split, particles.end() );

        if ( active && spawnRate > 0 )
            spawn( timeStep );
    }

    void update2( float timeStep )
    {
        auto split = std::remove_if( particles.begin(), particles.end(),
            []( Particle& p ) {
                return p.isExpired();
            } );

        particles.erase( split, particles.end() );

        if ( active && spawnRate > 0 )
            spawn( timeStep );
    }

    template< typename T >
    static T apply_variance( T value, float variance )
    {
        float r = variance * ((std::rand() - (RAND_MAX / 2)) / float( RAND_MAX / 2 ));
        return value + r;// *(1 + r);
    }

    static glm::vec2 apply_variance( glm::vec2 value, glm::vec2 variance )
    {
        value[ 0 ] = apply_variance( value[ 0 ], variance[ 0 ] );
        value[ 1 ] = apply_variance( value[ 1 ], variance[ 1 ] );
        return value;
    }

    static glm::vec3 apply_variance( glm::vec3 value, glm::vec3 variance )
    {
        value[ 0 ] = apply_variance( value[ 0 ], variance[ 0 ] );
        value[ 1 ] = apply_variance( value[ 1 ], variance[ 1 ] );
        value[ 2 ] = apply_variance( value[ 2 ], variance[ 2 ] );
        return value;
    }

    static glm::vec4 apply_variance( glm::vec4 value, glm::vec4 variance )
    {
        value[ 0 ] = apply_variance( value[ 0 ], variance[ 0 ] );
        value[ 1 ] = apply_variance( value[ 1 ], variance[ 1 ] );
        value[ 2 ] = apply_variance( value[ 2 ], variance[ 2 ] );
        value[ 3 ] = apply_variance( value[ 3 ], variance[ 3 ] );
        return value;
    }
};

class TestScene
    : public LevelScene
{
public:

    class EntityPlayerType
        : public EntityPlayer
    {
    public:

        TestScene* pScene;

        EntityPlayerType( int index = -1, TestScene* scene = 0 )
            : EntityPlayer( index )
            , pScene( scene )
        {
        }

        //void onDestroy( Entity2& ntt ) override
        //{
        //    EntityPlayer::onDestroy( ntt );
        //    if ( pScene != nullptr )
        //        pScene->_spawnParticle( ntt.position );
        //}

        void onEvent( int e ) override
        {
            EntityPlayer::onEvent( e );
            if ( e == 1 && player && player->respawning <= 0)
            {
                player->alive = false;
                EntityId eid { "player", (uint16_t) playerIndex };
                pScene->_spawnParticle( pScene->entities[ eid ].position );
            }
        }
    };

    std::vector< ParticleEmitter > emitters;

    //OpenCLKernel< Particle*, float > updater0;

    std::array< int, MAX_PLAYERS > controllerRedirect;

	TestScene( int width, int height,
               std::array< int, MAX_PLAYERS > playerDat )
		: LevelScene( width, height, "Audio/Banks/MasterBank",
                      MAX_PLAYERS - std::count(
                          begin( playerDat ),
                          end( playerDat ),
                          -1 ) )
        //, updater0( "ParticleSystem.cl" )
        , controllerRedirect( playerDat )
	{
	}

    void _spawnParticle( Vec2 position )
	{
        ParticleEmitter emitter( position );
        emitter.active = false;
        //emitter.spawnRate = 10;
        emitter.color = { 0.8, 0, 0, 0.9 };
        emitter.colorVariance = { 0.1, 0, 0, 0.1 };
        emitter.lifetime = 2.5;
        emitter.lifetimeVariance = 1;
        emitter.velocityMagnitude = 60;
        emitter.velocityMagnitudeVariance = 55;
        emitter.velocityAngle = 0;
        emitter.velocityAngleVariance = glm::pi< float >();

        emitter.fnUpdate = []( Particle& p, float timeStep ) {
            p.color.w -= timeStep / 3;
        };

        emitter.emitt( 500 );

        emitters.push_back( std::move( emitter ) );
	}

	void exit(unsigned ticks) override {
		LevelScene::exit(ticks);
		gameOver = false;
		Player::deadPlayers = 0;
		pAudioEngine->stopAllEvents();
	}

    //void updateEmitter( OpenCLKernel< Particle*, float >& updater, ParticleEmitter& em, float t )
    //{
    //    updater.globalWorkSize[ 0 ] = em.particles.size();
    //    updater( em.particles, t );
    //    em.update2( t );
    //}

    void update( unsigned ticks )
    {
        LevelScene::update( ticks );

        /*for ( int i = 0; i < emitters.size(); i += 4 )
        {
            int chunk_size = emitters.size() - i;

            std::thread* threads[ 4 ];

            switch ( chunk_size )
            {
            default: threads[ 3 ] = new std::thread( [this, i]() {
                updateEmitter( updater3, emitters[ i + 3 ], Scene::ticksDiff / 1000.f );
            } );
            case 3: threads[ 2 ] = new std::thread( [this, i]() {
                updateEmitter( updater2, emitters[ i + 2 ], Scene::ticksDiff / 1000.f );
            } );
            case 2: threads[ 1 ] = new std::thread( [this, i]() {
                updateEmitter( updater1, emitters[ i + 1 ], Scene::ticksDiff / 1000.f );
            } );
            case 1: threads[ 0 ] = new std::thread( [this, i]() {
                updateEmitter( updater0, emitters[ i + 0 ], Scene::ticksDiff / 1000.f );
            } );
            }

            for ( int i = 0; i < std::min( chunk_size - 1, 4 ); ++i )
            {
                threads[ i ]->join();
                delete threads[ i ];
            }
        }*/

        std::vector< std::future< void > > futures;
        futures.reserve( emitters.size() );

        float tDiff = Scene::ticksDiff / 1000.f;
        for ( auto& em : emitters )
        {
            futures.push_back( std::async( [&] { em.update( tDiff ); } ) );
            //updater0.settings.globalWorkSize[ 0 ] = em.particles.size();
            //futures.push_back( updater0( em.particles, tDiff ) );
        }

        for ( auto& fut : futures )
            fut.get();

        //for ( auto& em : emitters )
        //    em.update2( tDiff );
        
        auto itr = std::remove_if( emitters.begin(), emitters.end(),
            []( const ParticleEmitter& em ) {
                return !em.active && em.particles.empty();
            } );
        
        emitters.erase( itr, emitters.end() );
    }

    GraphicalComponent _gfx = GraphicalComponent::makeRect( 1, 1 );

    void draw()
    {
        LevelScene::draw();

        using namespace glm;
		
        glUseProgram( program );
        glUniform( uTexture, _gfx.texture );
        glUniform( uFacingDirection, _gfx.direction );

        glUniform( uCurrentAnim, 0.f );
        glUniform( uCurrentFrame, 0.f );
        glUniform( uMaxFrame, 1.f );
        glUniform( uMaxAnim, 1.f );

        glBindVertexArray( _gfx.vertexArray );

        for ( auto& emitter : emitters )
        {
            for ( Particle& p : emitter.particles )
            {
                if ( p.isExpired() )
                    continue;

                mat4 mtx = camera.getCameraMatrix()
                    * translate( {}, vec3( p.position.x, p.position.y, 0 ) );

                glUniform( uMatrix, mtx );
                glUniform( uColor, vec4( p.color.x, p.color.y, p.color.z, p.color.w ) );
                glDrawArrays( GL_TRIANGLES, 0, _gfx.count );
            }
        }
	}

	void init( unsigned ticks )
    {
        LevelScene::init( ticks );

        Entity2& bg = entities[ EntityId( 0 ) ];
        bg.pDrawable = newGraphics( GraphicalComponent::makeRect( width, height ) );
		bg.pDrawable->texture = BACKGROUND_ANIM;
		bg.pDrawable->interactive = false;
		bg.pAnimator = newAnimator(AnimatorComponent({ 9 }));
		bg.pAnimator->loop = false;
		bg.pAnimator->currentFrame = 0;

        listeners.push_back( [this]( const InputManager& inmn ) {
            if ( inmn.wasKeyPressed( SDLK_BACKSPACE ) )
                this->expired = true;
        } );

		listeners.push_back([this](const InputManager& inmn) {
			if (inmn.wasKeyPressed(SDLK_m))
				pAudioEngine->toggleMute();
		});

		listeners.push_back([this](const InputManager& inmn) {
			const float CAMERA_SPEED = 2.0f;
			const float SCALE_SPEED = 0.25f;
			if (inmn.wasKeyPressed(SDLK_e))
				camera.setScale(camera.getScale() + SCALE_SPEED);
			if (inmn.wasKeyPressed(SDLK_q))
				camera.setScale(camera.getScale() - SCALE_SPEED);
			if (inmn.wasKeyPressed(SDLK_w))
				camera.setPosition(camera.getPosition() + glm::vec2(0.0, CAMERA_SPEED));
			if (inmn.wasKeyPressed(SDLK_a))
				camera.setPosition(camera.getPosition() + glm::vec2(-CAMERA_SPEED, 0.0f));
			if (inmn.wasKeyPressed(SDLK_s))
				camera.setPosition(camera.getPosition() + glm::vec2(0.0, -CAMERA_SPEED));
			if (inmn.wasKeyPressed(SDLK_d))
				camera.setPosition(camera.getPosition() + glm::vec2(CAMERA_SPEED, 0.0f));
			if (inmn.wasKeyPressed(SDLK_p))
				camera.shake();
		});


		/*
		* Create Players. EntitityPlayers need to be updated to store player information so collision
		* resolution can use the data.
		*/
		// create player 1

        Vec2 startingPositions[] {
            { -22, 11.4f },
            { 22, 11.4f },
            { 22, -12 },
            { -22, -12 }
        };

        int i = 0;
        for ( int pno : controllerRedirect )
        {
            if ( pno != -1 )
            {
                uint16_t pidx = pno;
                odin::make_player( this, {"player", pidx}, startingPositions[ i ], pidx );
                EntityPlayer* ep = (EntityPlayer*) entities[ {"player", pidx} ].base();
                ep->player = &players[ pidx ];
                listeners.push_back( [this, pidx]( const InputManager& inmn ) {
                    return player_input( inmn, {"player", pidx}, pidx );
                } );
            }
            ++i;
        }

        //Setup level
		odin::make_platform(this, "plat01", 30, {-240 ,-144}); // bottom floor
		//odin::make_platform(this, "plat02", 6, { -48,-90 }); // center lower platform
		odin::make_platform(this, "plat03", 4, { -32,-40 }); // center mid-lower platform

		odin::make_platform(this, "plat04", 6, { -240,90 }); // left upper
		odin::make_platform(this, "plat05", 6, { 144,90}); // right upper
		odin::make_platform(this, "plat06", 6, { -48,68 }); // center upper

		odin::make_platform(this, "plat07", 3, { -148,-60 }); // right center
		odin::make_platform(this, "plat08", 3, { 100,-60 }); // right lower
		odin::make_platform(this, "plat09", 4, { -144,25 }); // left mid upper
		odin::make_platform(this, "plat10", 4, { 80, 25 }); // right mid upper

		odin::make_platform(this, "plat11", 4, { -240, -20 }); // left mid upper
		odin::make_platform(this, "plat12", 4, { 176, -20 }); // right mid upper

		odin::make_platform(this, "plat13", 3, { -240, -90 }); // left mid upper
		odin::make_platform(this, "plat14", 3, { 192, -90 }); // right mid upper

		odin::make_platform(this, "barr01", 1, { -72, -128 },BARREL); // barrel stack bottom left
		odin::make_platform(this, "barr02", 1, { -64, -112 }, BARREL); // barrel
		odin::make_platform(this, "barr03", 1, { -56, -128 }, BARREL); // barrel
		 
		odin::make_platform(this, "barr04", 1, { 56, -128 }, BARREL); // barrel stack bottom right
		odin::make_platform(this, "barr05", 1, { 48, -112 }, BARREL); // barrel
		odin::make_platform(this, "barr06", 1, { 40, -128 }, BARREL); // barrel

		odin::make_platform(this, "barr07", 1, {-16, 84 }, BARREL); // barrel stack top center
		odin::make_platform(this, "barr08", 1, { -8, 116  }, BARREL); // barrel top
		odin::make_platform(this, "barr09", 1, { -16, 100 }, BARREL); // barrel mid row
		odin::make_platform(this, "barr10", 1, { 0, 100 }, BARREL); // barrel
		odin::make_platform(this, "barr11", 1, { -8, 84 }, BARREL); // barrel bottom row
		odin::make_platform(this, "barr12", 1, { 8, 84 }, BARREL); // barrel
		odin::make_platform(this, "barr13", 1, { -24, 84 }, BARREL); // barrel

		// Set the physics bounds for the left,right wall and floor surfaces
		b2BodyDef floorDef;
		b2EdgeShape boundingShape;
		b2Filter wallFilter;
		boundingShape.Set({ -24, -14.5 }, { 24, -14.5 }); //floor plane

		wallFilter.categoryBits = PLATFORM;
		wallFilter.maskBits = PLAYER | HORSE | BULLET | DEAD;

        b2Body* floorBody = entities[ "floor" ].pBody = newBody( floorDef );

		b2Fixture* fix = floorBody->CreateFixture(&boundingShape, 1);
		fix->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);
		fix->SetFilterData(wallFilter);

		boundingShape.Set({ 24, 14.5 }, { 24, -14.5 }); //right wall plane

        b2Body* wallRBody = entities[ "wallR" ].pBody = newBody( floorDef );

		fix = wallRBody->CreateFixture(&boundingShape, 1);
		fix->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);
		fix->SetFilterData(wallFilter);

		boundingShape.Set({ -24, 14.5 }, { -24, -14.5 }); // left wall plane

        b2Body* wallLBody = entities[ "wallL" ].pBody = newBody( floorDef );

		fix = wallLBody->CreateFixture(&boundingShape, 1);
		fix->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);
		fix->SetFilterData(wallFilter);

		boundingShape.Set({ -24, 14.5 }, { 24, 14.5 }); //ceiling

		b2Body* ceilBody = entities[ "ceil" ].pBody = newBody( floorDef );
		fix = ceilBody->CreateFixture(&boundingShape, 1);
		fix->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);
		fix->SetFilterData(wallFilter);

		//load common events and play music
		pAudioEngine->loadEvent("event:/Music/EnergeticTheme");
		pAudioEngine->loadEvent("event:/Desperado/Shoot");
		pAudioEngine->loadEvent("event:/Desperado/Die");
		pAudioEngine->loadEvent("event:/Desperado/Ready");
		pAudioEngine->loadEvent("event:/Desperado/Draw");

		pAudioEngine->playEvent("event:/Music/EnergeticTheme");
		pAudioEngine->playEvent("event:/Desperado/Ready");
        #ifdef _DEBUG
        //pAudioEngine->toggleMute(); //mute audio
        #endif
	}

    void resume( unsigned tick ) override
    {
        odin::load_texture< GLubyte[4] >( NULL_TEXTURE, 1, 1, { 0xFF, 0xFF, 0xFF, 0xFF } );
        odin::load_texture(GROUND1, "Textures/ground.png");
        odin::load_texture(BARREL, "Textures/barrel.png");
        odin::load_texture(PLAYER1_TEXTURE, "Textures/CowboySS.png");
        odin::load_texture(PLAYER2_TEXTURE, "Textures/cowboy_r.png");
        odin::load_texture(PLAYER3_TEXTURE, "Textures/cowboy_g.png");
        odin::load_texture(PLAYER4_TEXTURE, "Textures/cowboy_b.png");
        odin::load_texture(ARM_TEXTURE, "Textures/ArmSS.png");
        odin::load_texture(BACKGROUND, "Textures/background.png");
        odin::load_texture(BULLET_TEXTURE, "Textures/bullet.png");
        odin::load_texture(WIN_TEXTURE, "Textures/win.png");
        odin::load_texture(READY_TEXTURE, "Textures/readytext.png");
        odin::load_texture(AMMO_COUNTER, "Textures/ammocounter.png");
        odin::load_texture(SKULL_COIN, "Textures/skullcoin.png");

        //background animation
        odin::load_texture(BACKGROUND_ANIM, "Textures/sunrisebg.png");
    }

};
