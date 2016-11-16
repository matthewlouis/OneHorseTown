// Andrew Meckling
#pragma once

#include <Odin/Scene.h>
#include <Odin/AudioEngine.h>
#include <Odin/ThreadedAudio.h>
#include <Odin/TextureManager.hpp>
#include <Odin/Camera.h>


#include "Constants.h"
#include "EntityFactory.h"
#include "Player.hpp"

#include <tuple>
#include <array>
#include <bitset>
#include <memory>

#include "ContextAllocator.hpp"
#include "TypedAllocator.hpp"

// Macros to speed up rendering
#define PI 3.1415926f
#define SIN45 0.7071f

using odin::Entity;
using odin::EntityId;
using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::AnimatorComponent;
using odin::InputManager;
using odin::InputListener;
using odin::ComponentType;
using odin::AudioEngine;

struct EntityView;
class Player;


// Defines a function which returns the component map for the 
// particular type of component. (This is a 'variadic macro')
#define OHT_DEFINE_COMPONENTS( ... )                            \
template< typename T >                                          \
auto& components()                                              \
{                                                               \
    return std::get< odin::BinarySearchMap< EntityId, T >& >(   \
        std::tie( __VA_ARGS__ ) );                              \
}

template< typename T, typename Sc >
auto& get_components( Sc* pScene )
{
    return pScene->components< T >();
}

struct Entity2;

class EntityBase
{
public:
    const char* name = "<no name>";

    virtual void onDestroy( Entity2& ) {}
} g_DEFAULT_ENTITY_BASE;

class MyContactListener : public b2ContactListener
{
public:

    std::vector< EntityBase* > deadEntities;

    std::function< void( Entity2* ) > func;

    void BeginContact(b2Contact* contact) {

        auto bodyA = contact->GetFixtureA()->GetBody();
        auto bodyB = contact->GetFixtureB()->GetBody();

        if ( bodyA->IsBullet() )
        {
            deadEntities.push_back( (EntityBase*) bodyA->GetUserData() );
            if ( bodyB->GetUserData() )
                deadEntities.push_back( (EntityBase*) bodyB->GetUserData() );
        }

        if ( bodyB->IsBullet() )
        {
            deadEntities.push_back( (EntityBase*) bodyB->GetUserData() );
            if ( bodyA->GetUserData() )
                deadEntities.push_back( (EntityBase*) bodyA->GetUserData() );
        }
    }

    void EndContact(b2Contact* contact) {

    }
};

struct Entity2
{
    friend class LevelScene;
    
    using SelfPointer = std::unique_ptr< EntityBase >;

    glm::vec2 position = { 0, 0 };
    float     rotation = 0;
    unsigned  flags    = 0;

    b2Body*             pBody     = nullptr;
    GraphicalComponent* pDrawable = nullptr;
    AnimatorComponent*  pAnimator = nullptr;

    Entity2() = default;

    Entity2( Vec2 pos, float rot, unsigned fl = 0 )
        : position( pos )
        , rotation( rot )
        , flags( fl )
    {
    }

    Entity2( Entity2&& move )
        : position( move.position )
        , rotation( move.rotation )
        , flags( move.flags )
        , pBody( move.pBody )
        , pDrawable( move.pDrawable )
        , pAnimator( move.pAnimator )
        , _self( std::move( move._self ) )
    {
        move.pBody     = nullptr;
        move.pDrawable = nullptr;
        move.pAnimator = nullptr;
    }

    ~Entity2() = default;

    Entity2& operator =( Entity2&& move )
    {
        position = move.position;
        rotation = move.rotation;
        flags = move.flags;
        std::swap( pBody, move.pBody );
        std::swap( pDrawable, move.pDrawable);
        std::swap( pAnimator, move.pAnimator );
        std::swap( _self, move._self );
        return *this;
    }

    void setBase( std::nullptr_t )
    {
        _self = nullptr;
    }

    void setBase( SelfPointer&& self )
    {
        _self = std::move( self );
    }

    template< typename EntityClass >
    void setBase( EntityClass self )
    {
        _self = std::make_unique< EntityClass >( std::move( self ) );
    }

    void reset()
    {
        _self = nullptr;
        pBody = nullptr;
        pDrawable = nullptr;
        pAnimator = nullptr;
    }

    void kill()
    {
        (*this)->onDestroy( *this );
        flags |= 1;
    }

    EntityBase* base()
    {
        return _self.get();
    }

    EntityBase* operator ->()
    {
        return _self ? _self.get() : &g_DEFAULT_ENTITY_BASE;
    }

private:

    SelfPointer _self = nullptr;
};


class EntityPlayer
    : public EntityBase
{
public:

    int playerIndex;

    EntityPlayer( int index = -1 )
        : EntityBase()
        , playerIndex( index )
    {
    }

    void onDestroy( Entity2& ntt ) override
    {
        std::cout << "Destroying player " << playerIndex << std::endl;
    }
};


class LevelScene
    : public odin::Scene
{
public:

    using EntityPlayerType = EntityPlayer;

    template< typename ValueType >
    using EntityMap = odin::BinarySearchMap< EntityId, ValueType >;

    static constexpr size_t COMP_MAX = 500;

    template< typename T >
    using Components = TypedAllocator< T, COMP_MAX >;

    EntityMap< Entity2 > entities;

    Components< GraphicalComponent > graphics;
    Components< AnimatorComponent >  animations;

    GraphicalComponent* newGraphics( GraphicalComponent gfx )
    {
        return ALLOC( graphics, GraphicalComponent )( std::move( gfx ) );
    }

    AnimatorComponent* newAnimator( AnimatorComponent anim )
    {
        return ALLOC( animations, AnimatorComponent )( std::move( anim ) );
    }

    b2Body* newBody( const b2BodyDef& bodyDef )
    {
        return b2world.CreateBody( &bodyDef );
    }

    void deleteComponent( GraphicalComponent* gfx )
    {
        DEALLOC( graphics, gfx );
    }

    void deleteComponent( AnimatorComponent* anim )
    {
        DEALLOC( animations, anim );
    }

    void deleteComponent( b2Body* body )
    {
        if ( body ) b2world.DestroyBody( body );
    }


    //EntityMap< Entity >             entities;

    //EntityMap< GraphicalComponent > gfxComponents;
    //EntityMap< AnimatorComponent >  animComponents;

    b2ThreadPool                    b2thd;
    b2World                         b2world = { { 0.f, -9.81f } };
    //EntityMap< PhysicalComponent >  fsxComponents;

    //OHT_DEFINE_COMPONENTS( entities, gfxComponents, animComponents, fsxComponents );

    InputManager*                   pInputManager;
    std::vector< InputListener >    listeners;

    std::string                     audioBankName;
    AudioEngine*                    pAudioEngine;

	odin::Camera camera;

    //SDL_Renderer* renderer;

    GLuint program;
    GLint uMatrix, uColor, uTexture, uFacingDirection,
        uCurrentFrame, uCurrentAnim, uMaxFrame, uMaxAnim;

    //for simulating energy - alpha presentation
    float energyLevel = 0;
    unsigned short _bulletCount = 0;

	// Range of the bullets, set to diagonal screen distance by default
	float bulletRange;

    MyContactListener _contactListener;

    /*PolymorphicAllocator<
        ThresholdAllocator< 4,
            BitsetAllocator< 1024 * 2, 4 >,
            ThresholdAllocator< 128,
                BitsetAllocator< 1024 * 1024, 8 >,
                Mallocator
            >
        >
    > _localAllocator;*/

	int			numberPlayers;
	Player      players[MAX_PLAYERS];

    LevelScene( int width, int height, std::string audioBank = "", int numberPlayers = MAX_PLAYERS )
        : Scene( width, height )
        , audioBankName( std::move( audioBank ) )
		, numberPlayers(numberPlayers)

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

		bulletRange = sqrt(width * width + height * height);
		camera.init(width, height);

        b2world.SetContactListener( &_contactListener );

        if ( audioBankName != "" )
        {
            pAudioEngine->loadBank( audioBankName + ".bank",
                                  FMOD_STUDIO_LOAD_BANK_NORMAL );
            pAudioEngine->loadBank( audioBankName + ".strings.bank",
                                  FMOD_STUDIO_LOAD_BANK_NORMAL );
        }
    }

    void resume( unsigned ticks )
    {
        Scene::resume( ticks );
        //context_allocator::push( _localAllocator );
    }

    void pause( unsigned ticks )
    {
        Scene::pause( ticks );
        //context_allocator::pop();
    }

    void exit( unsigned ticks )
    {
        Scene::exit( ticks );

        if ( audioBankName != "" )
        {
            pAudioEngine->unloadBank( audioBankName + ".bank" );
            pAudioEngine->unloadBank( audioBankName + ".strings.bank" );
        }
    }

    void _destroy( Entity2& ntt )
    {
        ntt.kill();
        deleteComponent( ntt.pBody );
        deleteComponent( ntt.pDrawable );
        deleteComponent( ntt.pAnimator );
    }

    void update( unsigned ticks )
    {
        Scene::update( ticks );

		for (int i = 0; i < this->numberPlayers; i++) {
			players[i].update();
		}

        for ( auto& lstn : listeners )
            lstn( *pInputManager );

        float timeStep = Scene::ticksDiff / 1000.f;
        b2world.Step( timeStep, 8, 3 );


        for ( auto x : entities )
        {
            decltype(auto) ntt = x.value;
            if ( auto body = ntt.pBody )
            {
                Vec2 pos = body->GetPosition();
                ntt.position = glm::round( glm::vec2( pos ) * 10.0f );
                ntt.rotation = body->GetAngle();
            }
        }

        std::vector< EntityId > deadEntities;
        deadEntities.reserve( _contactListener.deadEntities.size() );

        for ( auto x : entities )
        {
            decltype(auto) ntt = x.value;
            if ( auto animator = ntt.pAnimator )
            {
                animator->incrementFrame();

                switch ( animator->type )
                {
                    // Handle fading animation
                case odin::AnimationType::FADEOUT:
                    if (animator->currentFrame == animator->maxFrames-1)
                    {
                        _destroy( ntt );
                        deadEntities.push_back( x.key );
                    }
                    else {
                        ntt.pDrawable->color = { 1,1,1, 1.f - (float)animator->currentFrame
                                                            / (float)animator->maxFrames };
                    }
                default:
                    break;
                }
            }
        }

        for ( auto x : entities )
        {
            if ( auto pbase = x.value.base() )
            {
                for ( EntityBase* ebase : _contactListener.deadEntities )
                {
                    if ( ebase == pbase )
                    {
                        _destroy( x.value );
                        deadEntities.push_back( x.key );
                        //if ( _contactListener.func )
                        //    _contactListener.func( &x.value );
                        break;
                    }
                }
            }
        }

        if ( !_contactListener.deadEntities.empty() )
            _contactListener.deadEntities.clear();

        for ( EntityId eid : deadEntities )
            entities.remove( eid );
    }

    void draw()
    {
        using namespace glm;
        Scene::draw();

        //float zoom = 1.0f / SCALE;
        //float aspect = width / (float) height;
        //const mat4 base = scale( {}, vec3( zoom, zoom * aspect, 1 ) );
		//update camera matrix
		camera.update();
		glm::mat4 cameraMatrix = camera.getCameraMatrix();

        glUseProgram( program );

        for ( auto x : entities )
        {
            decltype(auto) ntt = x.value;
            if ( auto drawable = ntt.pDrawable )
            {
                if ( !drawable->visible )
                    continue;

                mat4 mtx = cameraMatrix * translate( {}, vec3( ntt.position, 0 ) );
                mtx = rotate( mtx, ntt.rotation, vec3( 0, 0, 1 ) );

                glUniform( uMatrix, mtx );
                glUniform( uColor, drawable->color );
                glUniform( uTexture, drawable->texture );
                glUniform( uFacingDirection, drawable->direction );

                if ( auto anim = ntt.pAnimator )
                {
                    glUniform( uCurrentAnim, (float) anim->animState );
                    glUniform( uCurrentFrame, (float) anim->currentFrame );
                    glUniform( uMaxFrame, (float) anim->maxFrames );
                    glUniform( uMaxAnim, (float) anim->totalAnim );
                }
                else
                {
                    glUniform( uCurrentAnim, 0.f );
                    glUniform( uCurrentFrame, 0.f );
                    glUniform( uMaxFrame, 1.f );
                    glUniform( uMaxAnim, 1.f );
                }

                glBindVertexArray( drawable->vertexArray );
                glDrawArrays( GL_TRIANGLES, 0, drawable->count );
            }
        }
    }

    /*void update2( unsigned ticks )
    {
        Scene::update( ticks );

        for ( auto& lstn : listeners )
            lstn( *pInputManager );

        float timeStep = Scene::ticksDiff / 1000.f;
        b2world.Step( timeStep, 8, 3 );
        for ( auto x : fsxComponents )
        {
            if ( x.value.pBody == nullptr || !x.value.pBody->IsActive() )
                continue;

            Entity& ntt = entities[ x.key ];
            auto& fsx = x.value;

            //ntt.position = fsx.position();
            // Round to remove blur/shimmer.
            ntt.position = glm::round( glm::vec2( fsx.position() ) * 10.f );

            //if ( fsx.pBody->IsBullet() )
            //{
            //    Vec2 vel = fsx.pBody->GetLinearVelocity();
            //
            //    float angle = std::atan2( vel.y, vel.x );
            //    fsx.pBody->SetTransform( ntt.position, angle );
            //}

            ntt.rotation = fsx.rotation();
            }

        for ( auto b : _contactListener.deadBodies )
        {
            for ( auto x : fsxComponents )
                if ( x.value.pBody == b )
                {
                    if ( _contactListener.func )
                        _contactListener.func( b );

                    entities.remove( x.key );
                    gfxComponents.remove( x.key );
                    fsxComponents.remove( x.key );
                    break;
        }
        }

        if ( !_contactListener.deadBodies.empty() )
            _contactListener.deadBodies.clear();

        for ( auto x : animComponents )
        {
            x.value.incrementFrame();
			auto& texAdjust = entities[x.key].texAdjust;

			texAdjust[0] = x.value.animState;
			texAdjust[1] = x.value.currentFrame;
			texAdjust[2] = x.value.maxFrames;
			texAdjust[3] = x.value.totalAnim;
			switch (x.value.type)
			{
			// Handle fading animation
			case odin::AnimationType::FADEOUT:
				if (x.value.currentFrame == x.value.maxFrames-1) {
					gfxComponents.remove(x.key);
					animComponents.remove(x.key);
					entities.remove(x.key);
				}
				else {
					gfxComponents[x.key].color = { 1,1,1, 1.f-(float)x.value.currentFrame / (float)x.value.maxFrames };
				}
			default:
				break;
			}
        }
    }

    void draw2()
    {
        using namespace glm;
        Scene::draw();

        //float zoom = 1.0f / SCALE;
        //float aspect = width / (float) height;
        //const mat4 base = scale( {}, vec3( zoom, zoom * aspect, 1 ) );
		const mat4 base = mat4();

		//update camera matrix
		camera.update();
		glm::mat4 cameraMatrix = camera.getCameraMatrix();

        glUseProgram( program );
        for ( auto x : gfxComponents )
        {
            if ( !x.value.visible )
                continue;

            Entity& ntt = entities[ x.key ];
            auto& gfx = x.value;

			if (!gfx.visible)
				continue;

            mat4 mtx = cameraMatrix * translate( base, vec3( ntt.position.glmvec2, 0 ) );
            mtx = rotate( mtx, ntt.rotation, vec3( 0, 0, 1 ) );

            glUniform( uMatrix, mtx );
            glUniform( uColor, gfx.color );
            glUniform( uTexture, gfx.texture );
            glUniform( uFacingDirection, gfx.direction );

            glUniform( uCurrentAnim, ntt.texAdjust[ 0 ] );
            glUniform( uCurrentFrame, ntt.texAdjust[ 1 ] );
            glUniform( uMaxFrame, ntt.texAdjust[ 2 ] );
            glUniform( uMaxAnim, ntt.texAdjust[ 3 ] );

            glBindVertexArray( gfx.vertexArray );
            glDrawArrays( GL_TRIANGLES, 0, gfx.count );
        }
    }*/

    void add( EntityId eid, GraphicalComponent gfx )
    {
        //gfxComponents.add( eid, std::move( gfx ) );
    }

    void add( EntityId eid, PhysicalComponent fsx )
    {
        //fsxComponents.add( eid, std::move( fsx ) );
    }

    void add( InputListener lstn )
    {
        listeners.push_back( std::move( lstn ) );
    }

    void player_input( const InputManager& mngr, EntityId eid, int pindex );

    // Using bullet start position, the velocity  direction, and default facing direction.

    void fireBullet( Vec2 position, odin::Direction8Way direction );

	// Casts a ray from position in the direction given.
	// returns eid, normal to the collision, and distance of collision
	std::tuple<EntityId, Vec2, float> resolveBulletCollision(Vec2 position, Vec2 direction);
};

/*struct EntityView
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

    void attach( AnimatorComponent fsx )
    {
        pScene->animComponents[ eid ] = std::move( fsx );
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
        case ComponentType::Animator:
            pScene->animComponents.remove( eid );
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

    AnimatorComponent* animComponent()
    {
        auto itr = pScene->animComponents.search( eid );
        return itr ? (AnimatorComponent*) itr : nullptr;
    }

};*/

inline void LevelScene::player_input( const InputManager& mngr, EntityId eid, int pindex )
{
    if ( !entities.search( eid ) )
        return;

    decltype(auto) ntt = entities[ eid ];

	//arm
    decltype(auto) arm_ntt = entities[ { "playes", (uint16_t)pindex } ];
	GraphicalComponent& arm_gfx = *arm_ntt.pDrawable;
	AnimatorComponent& arm_anim = *arm_ntt.pAnimator;

    b2Body& body = *ntt.pBody;
    GraphicalComponent& gfx = *ntt.pDrawable;
    AnimatorComponent& anim = *ntt.pAnimator;
    
	if(!players[pindex].active)
		players[pindex].init(&gfx, &anim, &body, &arm_gfx, &arm_anim, arm_ntt.pBody);

    Vec2 vel = body.GetLinearVelocity();
    float maxSpeed = 5.5f;
    float actionLeft = mngr.isKeyDown(SDLK_LEFT) ? 1.f : 0.f;
    float actionRight = mngr.isKeyDown(SDLK_RIGHT) ? 1.f : 0.f;

    //adjust facing direction FOR KEYBOARD ONLY
	if (actionLeft) {
		gfx.direction = odin::LEFT;
		arm_gfx.direction = odin::LEFT;
	}
	if (actionRight) {
		gfx.direction = odin::RIGHT;
		arm_gfx.direction = odin::RIGHT;
	}

    Vec2 aimDir = mngr.gamepads.joystickDir( pindex );
    aimDir.y = -aimDir.y;

    if ( glm::length( aimDir.glmvec2 ) < 0.25f )
        aimDir = {0, 0};

	//aim the arm graphics
	odin::Direction8Way aimDirection = players[pindex].aimArm(aimDir);

    if ( actionLeft == 0 && actionRight == 0 && aimDir.x == 0 )
    {
        //pFixt->SetFriction( 2 );
        vel.x = tween<float>(vel.x, 0, 12 * (1 / 60.0f));
    }
    else
    {
        //pFixt->SetFriction( 0 );
        vel.x += aimDir.x * (20 + 1) * (1 / 60.0); // for use w/gamepad

        vel.x -= actionLeft * (20 + 1) * (1 / 60.0f);
        vel.x += actionRight * (20 + 1) * (1 / 60.0f);
        vel.x = glm::clamp(vel.x, -maxSpeed, +maxSpeed);
    }

	
	if (mngr.wasKeyPressed(SDLK_UP)) {
		vel.y = 11;
	}

	if (mngr.wasKeyReleased(SDLK_UP) && vel.y > 0) {
		vel.y *= 0.6f;
	}

	if (mngr.gamepads.wasButtonPressed(pindex, SDL_CONTROLLER_BUTTON_A)) {
		vel.y = 11;
	}

	if (mngr.gamepads.wasButtonReleased(pindex, SDL_CONTROLLER_BUTTON_A) && vel.y > 0)
		vel.y *= 0.6f;

    // Handle Duck input on button X
    if (mngr.gamepads.wasButtonPressed(pindex, SDL_CONTROLLER_BUTTON_X))
    {

    }
    if (mngr.gamepads.wasButtonReleased(pindex, SDL_CONTROLLER_BUTTON_X))
    {

    }

    // Handle Shoot input on button B
    if (mngr.gamepads.wasButtonPressed(pindex, SDL_CONTROLLER_BUTTON_B))
    {
		arm_anim.play = true;
		arm_anim.currentFrame = 1;
		fireBullet( ntt.position, aimDirection);
    }
    if (mngr.gamepads.wasButtonReleased(pindex, SDL_CONTROLLER_BUTTON_B))
    {

    }
	
    //for testing audio
	if ( pindex == 0 && mngr.wasKeyPressed(SDLK_SPACE)) {
		playSound("Audio/FX/Shot.wav", 127);
		arm_anim.play = true;
		arm_anim.currentFrame = 1;
		fireBullet( ntt.position, aimDirection);
	}
    if (mngr.wasKeyPressed(SDLK_1))
        pAudioEngine->setEventParameter("event:/Music/EnergeticTheme", "Energy", 0.0); //low energy test
    if (mngr.wasKeyPressed(SDLK_2))
        pAudioEngine->setEventParameter("event:/Music/EnergeticTheme", "Energy", 1.0); //high energy test

	if (mngr.wasKeyPressed(SDLK_KP_8))
		pAudioEngine->changeMasterVolume(0.1);
	if (mngr.wasKeyPressed(SDLK_KP_2))
		pAudioEngine->changeMasterVolume(-0.1);

    body.SetLinearVelocity(vel);
}

// Casts ray from position in specified direction
// returns eid, normal to the collision, and distance from position
std::tuple<EntityId, Vec2, float> LevelScene::resolveBulletCollision(Vec2 position, Vec2 direction) {
	// buffer value
	float delta = 0.001;
	
	//set up input
	b2RayCastInput input;
	input.p1 = position;
    input.p2 = Vec2( position.glmvec2 + glm::normalize( direction.glmvec2 ) * bulletRange );
    //{
    //    position.x + direction.x * bulletRange, position.y + direction.y * bulletRange
    //};
	input.maxFraction = 1;

	//check every fixture of every body to find closest
	float closestFraction = 1; //start with end of line as p2
	b2Vec2 intersectionNormal(0, 0);

	EntityId eid;

    for ( b2Body* body = b2world.GetBodyList(); body; body = body->GetNext() )
    {
		for ( b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext() )
        {
			b2RayCastOutput output;
			if (!f->RayCast(&output, input, 0))
				continue;
			
            // TODO: This SHOULD filter out fixtues that don't collide with bullets... But doesn't seem to do so
			//if( (f->GetFilterData().maskBits & BULLET) )
			//	continue;

			if (output.fraction < closestFraction && output.fraction > delta)
            {
				closestFraction = output.fraction;
				intersectionNormal = output.normal;
				//eid = x.key;
			}
		}
		
	}
	return std::make_tuple( eid, intersectionNormal, (closestFraction * bulletRange) * 10 );

}

inline void LevelScene::fireBullet(Vec2 position, odin::Direction8Way direction)
{

	float length = 100.f;
	float rotation = 0;
	Vec2 offset = { 0,0 };
    //for alpha presentation, to simulate energy levels
    //more shots fired == more energy!
    if (energyLevel >= 1.0) {
        energyLevel = 1.0f;
    }
    else {
        energyLevel += 0.2f;
    }
    pAudioEngine->setEventParameter("event:/Music/EnergeticTheme", "Energy", energyLevel);
    pAudioEngine->playEvent("event:/Desperado/Shoot");

	// id of the entity hit, normal to the collision, distance to target
	std::tuple<EntityId, Vec2, float> collisionData;
	
	// Determine the offset and rotation of the bullet graphic based on the aim direction
    glm::vec2 pos = position;
    pos /= 10.0f;
	switch (direction) {
	case odin::NORTH_WEST:
		collisionData = resolveBulletCollision(pos, { -1,1 });
		length = std::get<2>(collisionData);
		rotation = -PI/4;
		offset = { SIN45 * -length/2, SIN45 * length/2 };
		break;
	case odin::NORTH_EAST:
		collisionData = resolveBulletCollision(pos, { 1,1 });
		length = std::get<2>(collisionData);
		rotation = PI / 4;
		offset = { SIN45 * length / 2, SIN45 * length / 2 };
		break;
	case odin::SOUTH_WEST:
		collisionData = resolveBulletCollision(pos, { -1,-1 });
		length = std::get<2>(collisionData);
		rotation = PI / 4;
		offset = { SIN45 * -length / 2, SIN45 * -length / 2 };
		break;
	case odin::SOUTH_EAST:
		collisionData = resolveBulletCollision(pos, { 1,-1 });
		length = std::get<2>(collisionData);
		rotation = -PI / 4;
		offset = { SIN45 * length / 2, SIN45 * -length / 2 };
		break;
	case odin::NORTH:
		collisionData = resolveBulletCollision(pos, { 0,1 });
		length = std::get<2>(collisionData);
		rotation = PI / 2;
		offset = { 0, length / 2 };
		break;
	case odin::SOUTH:
		collisionData = resolveBulletCollision(pos, { 0, -1 });
		length = std::get<2>(collisionData);
		rotation = -PI / 2;
		offset = { 0, -length / 2 };
		break;
	case odin::WEST:
		collisionData = resolveBulletCollision(pos, { -1,0 });
		length = std::get<2>(collisionData);
		offset = { -length / 2, 0 };
		break;
	case odin::EAST:
		collisionData = resolveBulletCollision(pos, { 1,0 });
		length = std::get<2>(collisionData);
		offset = { length / 2, 0 };
		break;
	default:
		break;
	}

    EntityId bid("bulleq", _bulletCount);

    decltype(auto) bullet = entities[ bid ];
    bullet.position = position;
    bullet.setBase( EntityBase {} );

    bullet.pDrawable = newGraphics( GraphicalComponent::makeRect( 1, 1, { 0, 0, 0 } ) );

    glm::vec2 off = offset;

    b2BodyDef bodyDef;
    bodyDef.position = Vec2( (position.glmvec2 / 10.f) + glm::normalize( off ) * 2.0f );
    bodyDef.linearVelocity = Vec2( glm::normalize( off ) * 500.0f );//{ 500, 0 };
    bodyDef.type = b2_dynamicBody;
    bodyDef.gravityScale = 0;
    bodyDef.bullet = true;
    bodyDef.userData = bullet.base();

    //bullet.pBody = newBody( bodyDef );
    auto circ = PhysicalComponent::makeCircle( 0.05, b2world, bodyDef );
    bullet.pBody = circ.pBody;
    circ.pBody = nullptr;

    EntityId eid("bullet", _bulletCount);

    decltype(auto) bullet2 = entities[ eid ];
    bullet2.position = Vec2( position + offset );
    bullet2.rotation = rotation;

	bullet2.pDrawable = newGraphics( GraphicalComponent::makeRect(length, 8.0f, { 255.f, 255.f, 255.f }));
    bullet2.pDrawable->texture = BULLET_TEXTURE;

    bullet2.pAnimator = newAnimator( AnimatorComponent( { 8 }, odin::FADEOUT ) );

    ++_bulletCount;

	camera.shake();

    //return EntityView(bid, this);

}

class TitleScene
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

	InputManager*                   pInputManager;
	std::vector< InputListener >    listeners;

	std::string                     audioBankName;
	AudioEngine*                    pAudioEngine;

	OHT_DEFINE_COMPONENTS(entities, gfxComponents, fsxComponents);

	//SDL_Renderer* renderer;

	EntityId promptID;

	// Variables for blinking animation
	unsigned OFF_TIME = 40, ON_TIME = 55;
	unsigned onFrame = 0, offFrame = 0;
	bool promptOn = true;
	bool buttonPressed = false;

	unsigned int TIME_BEFORE_INTRO = 10000;
	bool introStarted = false;
	int currentSlide = 0;
	int slideTimes[14] = { 0, 3000, 3000, 1500, 1500, 2500, 700, 700, 700, 2000, 500, 500, 500, 500 };
	GraphicalComponent* background;
	int fadeTime = 250;
	bool fadedOut = false;
	bool fading = false;
	bool goingBackToTitle = false;

	GLuint program;
	GLint uMatrix, uColor, uTexture, uFacingDirection,
		uCurrentFrame, uCurrentAnim, uMaxFrame, uMaxAnim,
		uFadeOut;

	TitleScene(int width, int height, std::string audioBank = "")
		: Scene(width, height)
		, audioBankName(std::move(audioBank))
		, program(load_shaders("Shaders/vertexAnim.glsl", "Shaders/fragmentShader.glsl"))
		, uMatrix(glGetUniformLocation(program, "uMatrix"))
		, uColor(glGetUniformLocation(program, "uColor"))
		, uTexture(glGetUniformLocation(program, "uTexture"))
		, uFacingDirection(glGetUniformLocation(program, "uFacingDirection"))
		, uCurrentFrame(glGetUniformLocation(program, "uCurrentFrame"))
		, uCurrentAnim(glGetUniformLocation(program, "uCurrentAnim"))
		, uMaxFrame(glGetUniformLocation(program, "uMaxFrames"))
		, uMaxAnim(glGetUniformLocation(program, "uTotalAnim"))
		, uFadeOut(glGetUniformLocation(program, "uFadeOut"))
	{
	}

	//fade out scene, and return true when complete, must be called every update
	bool fadeout(int startTime, int currentTime, int fadeLength) {
		int timePassed = currentTime - startTime;

		//linear fade
		float fadeAmount = (float)timePassed / fadeLength;
		fadeAmount = fadeAmount > 1.0f ? 1.0 : fadeAmount; //clamp if greater than 1;

		glUniform(uFadeOut, fadeAmount);

		if (fadeAmount >= 1.0f) {
			return true;
		}
		else {
			return false;
		}
	}

	//fade in scene, and return true when complete, must be called every update
	bool fadein(int startTime, int currentTime, int fadeLength) {
		int timePassed = currentTime - startTime;

		//linear fade
		float fadeAmount = 1.0f - (float)timePassed / fadeLength;
		fadeAmount = fadeAmount < 0.0f ? 0.0 : fadeAmount; //clamp if greater than 1;

		glUniform(uFadeOut, fadeAmount);

		if (fadeAmount <= 0.0f) {
			return true;
		}
		else {
			return false;
		}
	}

	void init(unsigned ticks)
	{
		Scene::init(ticks);

		odin::load_texture(TITLE, "Textures/title.png");
		odin::load_texture(PRESS_BUTTON, "Textures/pressbutton.png");
		odin::load_texture(INTRO_1, "Textures/Intro/1.png");
		odin::load_texture(INTRO_2, "Textures/Intro/2.png");
		odin::load_texture(INTRO_3A, "Textures/Intro/3a.png");
		odin::load_texture(INTRO_3B, "Textures/Intro/3b.png");
		odin::load_texture(INTRO_4, "Textures/Intro/4.png");
		odin::load_texture(INTRO_5A, "Textures/Intro/5a.png");
		odin::load_texture(INTRO_5B, "Textures/Intro/5b.png");
		odin::load_texture(INTRO_5C, "Textures/Intro/5c.png");
		odin::load_texture(INTRO_5D, "Textures/Intro/5d.png");
		odin::load_texture(INTRO_6, "Textures/Intro/6.png");
		odin::load_texture(INTRO_7, "Textures/Intro/7.png");
		odin::load_texture(INTRO_8, "Textures/Intro/8.png");

		background = gfxComponents.add(
			EntityId(0), GraphicalComponent::makeRect(width, height));
		background->texture = TITLE;
		
		promptID = EntityId(1);
		auto prompt = gfxComponents.add(promptID, GraphicalComponent::makeRect(110, 15));
		prompt->texture = PRESS_BUTTON;

		Vec2 pos = { 160, -55 };
		if (!entities.add(promptID, Entity(pos, 0)))
		   std::cout << "Entity " << promptID << " already exists.\n";

		listeners.push_back([this](const InputManager& inmn) {
			if (inmn.wasKeyPressed(SDL_CONTROLLER_BUTTON_START) || inmn.wasKeyPressed(SDLK_RETURN)) {
				buttonPressed = true;
			}
		});

		if (audioBankName != "")
		{
			pAudioEngine->loadBank(audioBankName + ".bank",
				FMOD_STUDIO_LOAD_BANK_NORMAL);
			pAudioEngine->loadBank(audioBankName + ".strings.bank",
				FMOD_STUDIO_LOAD_BANK_NORMAL);
		}
	}

	void exit(unsigned ticks)
	{
		Scene::exit(ticks);

		if (audioBankName != "")
		{
			pAudioEngine->unloadBank(audioBankName + ".bank");
			pAudioEngine->unloadBank(audioBankName + ".strings.bank");
		}
	}

	void update(unsigned ticks)
	{
		static int timeAtStartScreen = ticks;

		if (ticks - timeAtStartScreen > TIME_BEFORE_INTRO) {
			introStarted = true;
		}


		//Do slideshow intro
		if (introStarted) {
			static int slideStartTime = ticks;
			
			offFrame = 0;
			promptOn = false;

			if (SDL_GetTicks() - slideStartTime > slideTimes[currentSlide]) { //if enough time has passed
				
				static unsigned int fadeStartTime = ticks;
				if (!fading) {
					fadeStartTime = ticks;
					fading = true;
				}

				//check if slide has faded out, if so change the slide
				if (!fadedOut && fadeout(fadeStartTime, ticks, fadeTime)) {
					fadedOut = true;

					if (currentSlide > 12) {
						background->texture = TITLE;
						currentSlide = 0;
					}
					else {
						background->texture = INTRO_1 + currentSlide++;
					}			
					fadeStartTime = ticks; //reset fade start to do fade-in
				}
				else if (fadedOut && fadein(fadeStartTime, ticks, fadeTime)) {
					fadedOut = false;
					slideStartTime = ticks;

					fading = false;
					if (currentSlide == 0) { //we've just faded back into the title screen
						introStarted = false;
						timeAtStartScreen = slideStartTime;
					}
				}
			}
		}


		if (buttonPressed && !introStarted) {
			static int startedTicks = ticks;
			ON_TIME = OFF_TIME = 4;
			
			if (fadeout(startedTicks, ticks, 2000)) {
				this->expired = true;
			}
		}
		else if (buttonPressed && introStarted) {
			static int startedTicks = ticks;
			if (!goingBackToTitle) {
				startedTicks = ticks;
				goingBackToTitle = true;
			}

			if (fadeout(startedTicks, ticks, 500)) {
				//reset everything to original state to go back to title
				currentSlide = 0;
				introStarted = false;
				background->texture = TITLE;
				timeAtStartScreen = ticks;
				glUniform(uFadeOut, 0.0f);
				buttonPressed = false;
				goingBackToTitle = false;
				fading = false;
			}
		}

		Scene::update(ticks);

		for ( auto& lstn : listeners )
			lstn( *pInputManager );
	}

	void draw()
	{
		using namespace glm;
		Scene::draw();

		float zoom = 1.0f / SCALE;
		float aspect = width / (float)height;
		const mat4 base = scale({}, vec3(zoom, zoom * aspect, 1));

		if (promptOn) {
			++onFrame;
			if (onFrame > ON_TIME) {
				gfxComponents[promptID].color.w = 0;
				promptOn = false;
				onFrame = 0;
			}
		}else {
			++offFrame;
			if (offFrame > OFF_TIME) {
				gfxComponents[promptID].color.w = 1;
				promptOn = true;
				offFrame = 0;
			}
		}

		glUseProgram(program);
		for (auto x : gfxComponents)
		{

			Entity& ntt = entities[x.key];
			auto& gfx = x.value;

			if (!gfx.visible)
				continue;

			mat4 mtx = translate(base, vec3(ntt.position.glmvec2, 0));
			mtx = rotate(mtx, ntt.rotation, vec3(0, 0, 1));

			glUniform(uMatrix, mtx);
			glUniform(uColor, gfx.color);
			glUniform(uTexture, gfx.texture);
			glUniform(uFacingDirection, gfx.direction);

            glUniform( uCurrentAnim, ntt.texAdjust[ 0 ] );
            glUniform( uCurrentFrame, ntt.texAdjust[ 1 ] );
            glUniform( uMaxFrame, ntt.texAdjust[ 2 ] );
            glUniform( uMaxAnim, ntt.texAdjust[ 3 ] );

			glBindVertexArray(gfx.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, gfx.count);
		}
	}

	void add(EntityId eid, GraphicalComponent gfx)
	{
		gfxComponents.add(eid, std::move(gfx));
	}

	void add(EntityId eid, PhysicalComponent fsx)
	{
		fsxComponents.add(eid, std::move(fsx));
	}

	void add(InputListener lstn)
	{
		listeners.push_back(std::move(lstn));
	}
};