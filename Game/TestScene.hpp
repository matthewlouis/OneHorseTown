#pragma once

#include <Odin/SceneManager.hpp>
#include <Odin/TextureManager.hpp>

#include "EntityFactory.h"
#include "Scenes.hpp"

#include <functional>

#include "OpenCLKernel.h"

using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::InputManager;
using odin::Entity;
using odin::EntityId;
using odin::Scene;

struct ParticleDeleter
{
    unsigned deleteBy;

    unsigned short firstDelete;
    unsigned short lastDelete;
};

struct Particle
{
    unsigned  id;
    float     lifetime;
    cl_float4 color = { 1, 1, 1, 1 };
    cl_float2 position = { 0, 0 };
    cl_float2 velocity = { 0, 0 };
    
    Particle() = default;

    Particle( unsigned id, float lifetime, glm::vec4 color,
              glm::vec2 pos, glm::vec2 vel )
        : id( id )
        , lifetime( lifetime )
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
        //p.position += p.velocity * step;
    }
};

template< typename T >
struct Fuzzy
{
    T value;
    float variance = 0;

    T resolve() const
    {
        float r = variance * ((std::rand() - (RAND_MAX / 2)) / float( RAND_MAX / 2 ));
        return value * (1 + r);
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

    Particle* emitt()
    {
        static unsigned s_NextId = 0;

        float length = apply_variance( velocityMagnitude, velocityMagnitudeVariance );
        float angle = apply_variance( velocityAngle, velocityAngleVariance );

        glm::vec2 vel = {
            length * glm::cos( angle ),
            length * glm::sin( angle )
        };

        Particle particle( s_NextId++,
            apply_variance( lifetime, lifetimeVariance ),
            apply_variance( color, colorVariance ),
            position, vel );

        particles.push_back( particle );

        return &particles.back();
    }

    void update( float timeStep )
    {
        //for ( Particle* p : particles )
        //{
        //    p->velocity += glm::vec2( 0, -50 ) * timeStep;
        //    Particle::default_update( *p, timeStep );
        //}

        auto split = std::remove_if( particles.begin(), particles.end(),
            []( Particle& p ) {
                return p.isExpired();
            } );

        //if ( fnUpdate )
        //    std::for_each( particles.begin(), split,
        //        [this, timeStep]( Particle* p ) {
        //            fnUpdate( *p, timeStep );
        //        } );

        //std::for_each( split, particles.end(),
        //    []( Particle* p ) {
        //        context_dealloc( p );
        //    } );

        particles.erase( split, particles.end() );

        if ( !active )
            return;

        float numToEmitt = std::rand() / float( RAND_MAX ) * spawnRate;
        numToEmitt = glm::round( numToEmitt * timeStep * 10 );

        while ( numToEmitt-- > 0 )
            emitt();
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

class ParticleSystem
{
public:

    std::vector< ParticleEmitter > emitters;

    void update( float timeStep )
    {
        for ( auto& emitter : emitters )
            emitter.update( timeStep );


    }
};

class TestScene
    : public LevelScene
{
public:

	EntityFactory* factory;

	float _scale;

    std::vector< ParticleEmitter > emitters;

    OpenCLKernel< Particle*, float > updater;

	TestScene( int width, int height, float scale )
		: LevelScene( width, height, "Audio/Banks/MasterBank" )
        , factory( EntityFactory::instance() )
		, _scale( scale )
        , updater( "ParticleSystem.cl" )
	{
        srand((unsigned)time(NULL));
	}

    #define PARTICLE_TAG "playez"

    void _spawnParticle( Vec2 position )
    {
        ParticleEmitter emitter( position );
        emitter.active = false;
        //emitter.spawnRate = 20;
        emitter.color = { 0.9, 0, 0, 1 };
        emitter.colorVariance = { 0.1, 0, 0, 0 };
        emitter.lifetime = 3;
        emitter.lifetimeVariance = 0.3;
        emitter.velocityMagnitude = 40;
        emitter.velocityMagnitudeVariance = 30;
        //emitter.velocityAngle = (std::rand() / float( RAND_MAX )) * glm::two_pi< float >();
        //emitter.velocityAngleVariance = glm::pi< float >() / 6;

        emitter.velocityAngle = 0;
        emitter.velocityAngleVariance = glm::pi< float >();

        emitter.fnUpdate = []( Particle& p, float timeStep ) {
            //p.color.w -= timeStep / 3;
        };

        for ( size_t i = 0; i < 50; ++i )
            emitter.emitt();

        emitters.push_back( emitter );
    }

    void update( unsigned ticks )
    {
        LevelScene::update( ticks );


        for ( auto& em : emitters )
        {
            updater.globalWorkSize[ 0 ] = em.particles.size();

            updater( em.particles, Scene::ticksDiff / 1000.f );
        }

        for ( auto& emitter : emitters )
            emitter.update( Scene::ticksDiff / 1000.f );
        
        auto itr = std::remove_if( emitters.begin(), emitters.end(),
            []( const ParticleEmitter& em ) {
                return !em.active && em.particles.empty();
            } );
        
        emitters.erase( itr, emitters.end() );
    }

    void draw()
    {
        LevelScene::draw();

        GraphicalComponent gfx = GraphicalComponent::makeRect( 1, 1 );

        using namespace glm;

        glUseProgram( program );
        glUniform( uTexture, gfx.texture );
        glUniform( uFacingDirection, gfx.direction );

        glUniform( uCurrentAnim, 0 );
        glUniform( uCurrentFrame, 0 );
        glUniform( uMaxFrame, 1 );
        glUniform( uMaxAnim, 1 );

        glBindVertexArray( gfx.vertexArray );

        for ( auto& emitter : emitters )
        {
            for ( Particle& p : emitter.particles )
            {
                mat4 mtx = camera.getCameraMatrix()
                    * translate( {}, vec3( p.position.x, p.position.y, 0 ) );

                glUniform( uMatrix, mtx );
                glUniform( uColor, vec4( p.color.x, p.color.y, p.color.z, p.color.w ) );
                glDrawArrays( GL_TRIANGLES, 0, gfx.count );
            }
        }
    }

    void _registerEntities( EntityId first, EntityId last )
    {
        for ( auto eid = first; eid < last; ++eid._bitPattern )
        {
            entities[ eid ];
            gfxComponents[ eid ];
            fsxComponents[ eid ];
        }
    }

	void init( unsigned ticks )
    {
        LevelScene::init( ticks );

        //_registerEntities( {PARTICLE_TAG, 0}, {PARTICLE_TAG, 100} );

        #undef PARTICLE_TAG

		odin::load_texture(GROUND1, "Textures/ground.png");
		odin::load_texture(GROUND2, "Textures/ground2.png");
		odin::load_texture(PLAYER_TEXTURE, "Textures/CowboySS.png");
		odin::load_texture(BACKGROUND, "Textures/background.png");
		odin::load_texture(HORSE_TEXTURE, "Textures/horse_dense.png");
        //odin::load_texture< GLubyte[ 4 ] >( BLOOD, 1, 1, { 0xff, 0x00, 0x00, 0xff } );

		auto background = gfxComponents.add(
			EntityId(0), GraphicalComponent::makeRect( width, height ));
		background->texture = BACKGROUND;

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
		});

        //factory->makePlayer( this, {"player", 0} );
        odin::make_player( this, {"player", 0}, {0, 5} );
        listeners.push_back( [this]( const InputManager& inmn ) {
            return player_input( inmn, {"player", 0}, 0 );
        } );

        listeners.push_back( [this]( const InputManager& inmn ) {
            if ( inmn.wasKeyPressed( SDLK_p ) )
                _spawnParticle( entities[ "player" ].position );
        } );

		//factory->makeHorse(this, "horse");
        odin::make_horse( this, "horse", {0.0f, 0.0f} );

		odin::make_platform(this, "plat01", 1, { 0,0 });

		/*
		factory->makePlatform(this, "plat1", 3, {0, -3}); // Lower Middle
		factory->makePlatform(this, "plat2", 6, { 0.5, 3 }); // Upper middle
		
		factory->makePlatform(this, "plat3", 4, { -9, 5 }); // Top Left
		factory->makePlatform(this, "plat4", 4, { 10, 5 }); // Top Right
		
		factory->makePlatform(this, "plat5", 5, { -6, 0 }); // Middle Left
		factory->makePlatform(this, "plat6", 5, { 6, 0 }); // Middle Right
		
		factory->makePlatform(this, "plat7", 2, { -3, -5.5 }); // Lower Left
		factory->makePlatform(this, "plat8", 2, { 3, -5.5 }); // Lower Right
		*/

		//factory->makeRect(this, "box", { 1,1 }, { 1,1 }, 0, { 1,1,1 });

		//fireBullet({ -170, 5.5f }, { 100, 0 });

		//GLuint nul = load_texture( "null.png", 0 );

		b2BodyDef floorDef;
		b2EdgeShape floorShape;
		b2Filter wallFilter;
		
		floorShape.Set({ -11, -8 }, { 11, -8 });

		wallFilter.categoryBits = PLATFORM;
		wallFilter.maskBits = PLAYER | HORSE;

		fsxComponents["floor"] = b2world.CreateBody(&floorDef);
		b2Fixture* fix = fsxComponents["floor"]->CreateFixture(&floorShape, 1);
		fix->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);
		fix->SetFilterData(wallFilter);

		floorShape.Set({ 11, +10 }, { 11, -8 });

		fsxComponents["wallR"] = b2world.CreateBody(&floorDef);
		fix = fsxComponents["wallR"]->CreateFixture(&floorShape, 1);
		fix->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);
		fix->SetFilterData(wallFilter);

		floorShape.Set({ -11, +10 }, { -11, -8 });

		fsxComponents["wallL"] = b2world.CreateBody(&floorDef);
		fix = fsxComponents["wallL"]->CreateFixture(&floorShape, 1);
		fix->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);
		fix->SetFilterData(wallFilter);

		//load common events and play music
		pAudioEngine->loadEvent("event:/Music/EnergeticTheme");
		pAudioEngine->loadEvent("event:/Desperado/Shoot");

		pAudioEngine->playEvent("event:/Music/EnergeticTheme");
        pAudioEngine->toggleMute(); //mute audio
	}


};
