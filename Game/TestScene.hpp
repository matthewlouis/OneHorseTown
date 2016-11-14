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

	EntityView* players;
	EntityView* player_arms;
	int numberPlayers;

	//'table' to store offsets for placing arm - 1 for each animation state
	Vec2 armOffsets[5];

	float _scale;

    std::vector< ParticleEmitter > emitters;

    OpenCLKernel< Particle*, float > updater;

	TestScene( int width, int height, float scale, int numberPlayers = 1 )
		: LevelScene( width, height, "Audio/Banks/MasterBank" )
        , factory( EntityFactory::instance() )
        , numberPlayers( numberPlayers )
		, _scale( scale )
        , updater( "ParticleSystem.cl" )
	{
        srand((unsigned)time(NULL));
        players = (EntityView*)malloc(sizeof(EntityView) * numberPlayers);
        player_arms = (EntityView*)malloc(sizeof(EntityView) * numberPlayers);
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
        //set arm offsets so it renders in correct location
		armOffsets[0] = Vec2(0.5, 0.9);
		armOffsets[1] = Vec2(0.6, 0.75);
		armOffsets[2] = Vec2(0.75, 0.475);
		armOffsets[3] = Vec2(0.6, 0.275);
		armOffsets[4] = Vec2(0.25, 0.05);


        //_registerEntities( {PARTICLE_TAG, 0}, {PARTICLE_TAG, 100} );

        #undef PARTICLE_TAG

		odin::load_texture(GROUND1, "Textures/ground.png");
		odin::load_texture(GROUND2, "Textures/ground2.png");
		odin::load_texture(PLAYER_TEXTURE, "Textures/CowboySS.png");
		odin::load_texture(ARM_TEXTURE, "Textures/ArmSS.png");
		odin::load_texture(BACKGROUND, "Textures/background.png");
		odin::load_texture(HORSE_TEXTURE, "Textures/horse_dense.png");
		odin::load_texture(BULLET_TEXTURE, "Textures/bullet.png");


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
			if (inmn.wasKeyPressed(SDLK_p))
				camera.shake();

		});

        //factory->makePlayer( this, {"player", 0} );

		// create player 1
        odin::make_player( this, {"player", 0}, {0, 5}, 0 );
        listeners.push_back( [this]( const InputManager& inmn ) {
            return player_input( inmn, {"player", 0}, 0 );
        } );
		players[0] = EntityView({ "player", 0 }, this);
		player_arms[0] = EntityView({ "playes", 0 }, this);
		// create player 2
		odin::make_player(this, { "player", 1 }, { 0, 5 },1);
		listeners.push_back([this](const InputManager& inmn) {
			return player_input(inmn, { "player", 1 }, 1);
		});
		players[1] = EntityView({ "player", 1 }, this);
		player_arms[1] = EntityView({ "playes", 1 }, this);
		// create player 3
		odin::make_player(this, { "player", 2 }, { 0, 5 }, 2);
		listeners.push_back([this](const InputManager& inmn) {
			return player_input(inmn, { "player", 2 }, 2);
		});
		players[2] = EntityView({ "player", 2 }, this);
		player_arms[2] = EntityView({ "playes", 2 }, this);
		// create player 4
		odin::make_player(this, { "player", 3 }, { 0, 5 }, 3);
		listeners.push_back([this](const InputManager& inmn) {
			return player_input(inmn, { "player", 3 }, 3);
		});
		players[3] = EntityView({ "player", 3 }, this);
		player_arms[3] = EntityView({ "playes", 3 }, this);

        listeners.push_back( [this]( const InputManager& inmn ) {
            if ( inmn.wasKeyPressed( SDLK_b ) )
                _spawnParticle( entities[ "player" ].position );
        } );

		//factory->makeHorse(this, "horse");
        odin::make_horse( this, "horse", {0.0f, 5.f} );


		//starting left top to bottom right
		odin::make_platform(this, "plat06", 4, { -103,25 }); // left upper
		odin::make_platform(this, "plat01", 4, { -123,-10 }); // left mid
		odin::make_platform(this, "plat05", 4, { -103,-45 }); // left lower
		odin::make_platform(this, "plat07", 4, { 73,25 }); // right upper
		odin::make_platform(this, "plat02", 4, { 93,-10 }); // right center
		odin::make_platform(this, "plat08", 4, { 73,-45 }); // right lower
		odin::make_platform(this, "plat03", 5, { -25,-20 }); // center
		odin::make_platform(this, "plat04", 26, { -123, -80 }); // bottom floor

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


		// Set the physics bounds for the left,right wall and floor surfaces
		b2BodyDef floorDef;
		b2EdgeShape boundingShape;
		b2Filter wallFilter;
		boundingShape.Set({ -13, -8 }, { 13, -8 }); //floor plane
		
		wallFilter.categoryBits = PLATFORM;
		wallFilter.maskBits = PLAYER | HORSE | BULLET;

		fsxComponents["floor"] = b2world.CreateBody(&floorDef);
		b2Fixture* fix = fsxComponents["floor"]->CreateFixture(&boundingShape, 1);
		fix->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);
		fix->SetFilterData(wallFilter);

		boundingShape.Set({ 13, +10 }, { 13, -8 }); //right wall plane

		fsxComponents["wallR"] = b2world.CreateBody(&floorDef);
		fix = fsxComponents["wallR"]->CreateFixture(&boundingShape, 1);
		fix->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);
		fix->SetFilterData(wallFilter);

		boundingShape.Set({ -13, +10 }, { -13, -8 }); // left wall plane

		fsxComponents["wallL"] = b2world.CreateBody(&floorDef);
		fix = fsxComponents["wallL"]->CreateFixture(&boundingShape, 1);
		fix->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);
		fix->SetFilterData(wallFilter);

		boundingShape.Set({ -13, 8 }, { 13, 8 }); //ceiling

		fsxComponents["ceil"] = b2world.CreateBody(&floorDef);
		fix = fsxComponents["ceil"]->CreateFixture(&boundingShape, 1);
		fix->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);
		fix->SetFilterData(wallFilter);


		//load common events and play music
		pAudioEngine->loadEvent("event:/Music/EnergeticTheme");
		pAudioEngine->loadEvent("event:/Desperado/Shoot");

		pAudioEngine->playEvent("event:/Music/EnergeticTheme");
        pAudioEngine->toggleMute(); //mute audio
	}


	void update(unsigned ticks)
	{
		//iterate through all the arms and place them relative to the player using offsets
		for (int i = 0; i < numberPlayers; ++i) {
			Vec2 armPosition = players[i].fsxComponent()->position();
			
			//current state determines the arm offset
			int currentState = player_arms[i].animComponent()->animState;

			armPosition.y += armOffsets[currentState].y;
			armPosition.x += players[i].gfxComponent()->direction * armOffsets[currentState].x;

			player_arms[i].fsxComponent()->pBody->SetTransform(armPosition, 0);
		}

		LevelScene::update(ticks);
	}
};
