// Andrew Meckling
#pragma once

#include <Odin/Scene.h>
#include <Odin/AudioEngine.h>
#include <Odin/ThreadedAudio.h>
#include <Odin/TextureManager.hpp>
#include <Odin/Camera.h>


#include "Constants.h"
#include "EntityFactory.h"

#include <tuple>

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

class LevelScene
    : public odin::Scene
{
public:

    template< typename ValueType >
    using EntityMap = odin::BinarySearchMap< EntityId, ValueType >;

    EntityMap< Entity >             entities;

    EntityMap< GraphicalComponent > gfxComponents;
    EntityMap< AnimatorComponent >  animComponents;

    b2ThreadPool                    b2thd;
    b2World                         b2world = { { 0.f, -9.81f }, &b2thd };
    EntityMap< PhysicalComponent >  fsxComponents;

    OHT_DEFINE_COMPONENTS( entities, gfxComponents, animComponents, fsxComponents );

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

		camera.init(width, height);

        if ( audioBankName != "" )
        {
            pAudioEngine->loadBank( audioBankName + ".bank",
                                  FMOD_STUDIO_LOAD_BANK_NORMAL );
            pAudioEngine->loadBank( audioBankName + ".strings.bank",
                                  FMOD_STUDIO_LOAD_BANK_NORMAL );
        }
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

    void update( unsigned ticks )
    {
        Scene::update( ticks );

        for ( auto& lstn : listeners )
            lstn( *pInputManager );

        float timeStep = Scene::ticksDiff / 1000.f;
        b2world.Step( timeStep, 8, 3 );
        for ( auto x : fsxComponents )
        {
            Entity& ntt = entities[ x.key ];
            auto& fsx = x.value;

            //ntt.position = fsx.position();
            // Round to remove blur/shimmer.
            ntt.position = glm::round( glm::vec2( fsx.position() ) * 10.f );

            if ( fsx.pBody->IsBullet() )
            {
                Vec2 vel = fsx.pBody->GetLinearVelocity();

                float angle = std::atan2( vel.y, vel.x );
                fsx.pBody->SetTransform( ntt.position, angle );
            }

            ntt.rotation = fsx.rotation();
        }

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
			case odin::AnimationType::FADEOUT:
				if (x.value.currentFrame == x.value.maxFrames) {
					gfxComponents.remove(x.key);
					animComponents.remove(x.key);
				}
				else {
					gfxComponents[x.key].color = { 1,1,1, (float)x.value.currentFrame / (float)x.value.maxFrames };
				}
			default:
				break;
			}
        }
    }

    void draw()
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
            Entity& ntt = entities[ x.key ];
            auto& gfx = x.value;

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
    }

    void add( EntityId eid, GraphicalComponent gfx )
    {
        gfxComponents.add( eid, std::move( gfx ) );
    }

    void add( EntityId eid, PhysicalComponent fsx )
    {
        fsxComponents.add( eid, std::move( fsx ) );
    }

    void add( InputListener lstn )
    {
        listeners.push_back( std::move( lstn ) );
    }

    void player_input( const InputManager& mngr, EntityId eid, int pindex );

    // Using bullet start position, the velocity  direction, and default facing direction.

    EntityView fireBullet( Vec2 position, odin::FacingDirection direction );
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

};

inline void LevelScene::player_input( const InputManager& mngr, EntityId eid, int pindex )
{
    EntityView ntt = EntityView(eid, this);

    b2Body& body = *ntt.fsxComponent()->pBody;
    GraphicalComponent& gfx = *ntt.gfxComponent();
    AnimatorComponent& anim = *ntt.animComponent();

    Vec2 vel = body.GetLinearVelocity();
    float maxSpeed = 5.5f;
    float actionLeft = mngr.isKeyDown(SDLK_LEFT) ? 1.f : 0.f;
    float actionRight = mngr.isKeyDown(SDLK_RIGHT) ? 1.f : 0.f;

    //adjust facing direction
    if (actionLeft)
        gfx.direction = odin::LEFT;
    if (actionRight)
        gfx.direction = odin::RIGHT;

    Vec2 aimDir = mngr.gamepads.joystickDir( pindex );
    aimDir.y = -aimDir.y;

    if ( glm::length( aimDir.glmvec2 ) < 0.25f )
        aimDir = {0, 0};

    //adjust facing direction for joystick
    if (aimDir.x < 0)
        gfx.direction = odin::LEFT;
    if (aimDir.x > 0)
        gfx.direction = odin::RIGHT;

    //b2Fixture* pFixt = body.GetFixtureList();

    if ( actionLeft == 0 && actionRight == 0 && aimDir.x == 0 )
    {
        //pFixt->SetFriction( 2 );
        vel.x = tween<float>(vel.x, 0, 12 * (1 / 60.0f));
        anim.switchAnimState(0); //idle state
    }
    else
    {
        //pFixt->SetFriction( 0 );
        vel.x += aimDir.x * (20 + 1) * (1 / 60.0); // for use w/gamepad

        vel.x -= actionLeft * (20 + 1) * (1 / 60.0f);
        vel.x += actionRight * (20 + 1) * (1 / 60.0f);
        vel.x = glm::clamp(vel.x, -maxSpeed, +maxSpeed);
        anim.switchAnimState(1); //running
    }

    if (mngr.wasKeyPressed(SDLK_UP)) {
        vel.y = 11;
    }

    if (mngr.wasKeyReleased(SDLK_UP) && vel.y > 0) {
        vel.y *= 0.6f;
    }

    if (mngr.gamepads.wasButtonPressed(pindex, SDL_CONTROLLER_BUTTON_A))
        vel.y = 11;

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
        fireBullet(entities[ eid ].position, gfx.direction);
    }
    if (mngr.gamepads.wasButtonReleased(pindex, SDL_CONTROLLER_BUTTON_B))
    {

    }
	
    //for testing audio
    if (mngr.wasKeyPressed(SDLK_SPACE))
		playSound("Audio/FX/Shot.wav", 127);
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

inline EntityView LevelScene::fireBullet(Vec2 position, odin::FacingDirection direction)
{

	float rotation = 0;
	float length = 100;

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

	switch (direction) {
	case odin::LEFT:
		position.x -= length / 2;
		break;
	case odin::RIGHT:
		position.x += length / 2;
		break;
	default:
		break;
	}

	EntityId eid("bullet", _bulletCount++);

	if (!entities.add(eid, Entity(position, rotation)))
		std::cout << "Entity " << eid << " already exists.\n";

	if (!gfxComponents.add(eid, GraphicalComponent::makeRect(length, 2.f)))
		std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

	// WHAT SHOULD THESE VALUES BE?
	if (!animComponents.add(eid, AnimatorComponent({ 1, 10, 3 }, odin::AnimationType::FADEOUT)))
		std::cout << "Entity " << eid << " already has an AnimatorComponent.\n";


	/*
	velocity.x *= speed;
	velocity.y *= speed;

    if (!entities.add(eid, Entity(position, 0)))
        std::cout << "Entity " << eid << " already exists.\n";

    if (!gfxComponents.add(eid, GraphicalComponent::makeRect(10.f, 2.f)))
        std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

    b2BodyDef bodyDef;
    bodyDef.position = position;
    bodyDef.linearVelocity = velocity;
    bodyDef.type = b2_dynamicBody;
    bodyDef.bullet = true;

    if (!fsxComponents.add(eid, PhysicalComponent::makeCircle(.05f, b2world, bodyDef, 0.01f, BULLET, 0)))
        std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";
	*/
    return EntityView(eid, this);

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

	GLuint program;
	GLint uMatrix, uColor, uTexture, uFacingDirection,
		uCurrentFrame, uCurrentAnim, uMaxFrame, uMaxAnim;

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
	{
	}

	void init(unsigned ticks)
	{
		Scene::init(ticks);

		odin::load_texture(TITLE, "Textures/title.png");
		odin::load_texture(PRESS_BUTTON, "Textures/pressbutton.png");

		auto background = gfxComponents.add(
			EntityId(0), GraphicalComponent::makeRect(width, height));
		background->texture = TITLE;
		
		promptID = EntityId(1);
		auto prompt = gfxComponents.add(promptID, GraphicalComponent::makeRect(75, 10));
		prompt->texture = PRESS_BUTTON;

		Vec2 pos = { 87, -30 };
		if (!entities.add(promptID, Entity(pos, 0)))
		   std::cout << "Entity " << promptID << " already exists.\n";

		listeners.push_back([this](const InputManager& inmn) {
			if (inmn.wasKeyPressed(SDL_CONTROLLER_BUTTON_START) || inmn.wasKeyPressed(SDLK_RETURN))
				this->expired = true;
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