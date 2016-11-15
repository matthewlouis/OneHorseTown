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

	// Range of the bullets, set to diagonal screen distance by default
	unsigned bulletRange;

	Player      players[4];

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

		bulletRange = sqrt(width * width + height * height);
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
		
		for (int i = 0; i < 1; i++) {
			players[i].update();
		}

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

    EntityView fireBullet( Vec2 position, odin::Direction8Way direction );

	// Casts a ray from position in the direction given.
	// returns eid, normal to the collision, and distance of collision
	std::tuple<EntityId, Vec2, float> resolveBulletCollision(Vec2 position, Vec2 direction);
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

	//arm
	EntityView arm_ntt = EntityView({ "playes", (uint16_t)pindex }, this);
	GraphicalComponent& arm_gfx = *arm_ntt.gfxComponent();
	AnimatorComponent& arm_anim = *arm_ntt.animComponent();

    
    GraphicalComponent& gfx = *ntt.gfxComponent();
    AnimatorComponent& anim = *ntt.animComponent();
	PhysicalComponent& psx = *ntt.fsxComponent();
	b2Body& body = *ntt.fsxComponent()->pBody;

	if(!players[pindex].active)
		players[pindex].init(&gfx, &anim, &psx, &arm_gfx, &arm_anim, &*arm_ntt.fsxComponent());

    Vec2 vel = body.GetLinearVelocity();
    float maxSpeed = 5.5f;
    float actionLeft = mngr.isKeyDown(SDLK_LEFT) ? 1.f : 0.f;
    float actionRight = mngr.isKeyDown(SDLK_RIGHT) ? 1.f : 0.f;

    //adjust facing direction
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
		arm_gfx.visible = false;
    }
    else
    {
        //pFixt->SetFriction( 0 );
        vel.x += aimDir.x * (20 + 1) * (1 / 60.0); // for use w/gamepad

        vel.x -= actionLeft * (20 + 1) * (1 / 60.0f);
        vel.x += actionRight * (20 + 1) * (1 / 60.0f);
        vel.x = glm::clamp(vel.x, -maxSpeed, +maxSpeed);
		
		arm_gfx.visible = true; //show arm when running
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
		fireBullet(entities[{ "playes", (uint16_t)pindex }].position, aimDirection);
    }
    if (mngr.gamepads.wasButtonReleased(pindex, SDL_CONTROLLER_BUTTON_B))
    {

    }
	
    //for testing audio
	if (mngr.wasKeyPressed(SDLK_SPACE)) {
		playSound("Audio/FX/Shot.wav", 127);
		arm_anim.play = true;
		arm_anim.currentFrame = 1;
		fireBullet(entities[{ "playes", 0 }].position, aimDirection);
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
	input.p2 = { position.x + direction.x * bulletRange, position.y + direction.y * bulletRange };
	input.maxFraction = 1;

	//check every fixture of every body to find closest
	float closestFraction = 1; //start with end of line as p2
	b2Vec2 intersectionNormal(0, 0);

	EntityId eid;

	for (auto x : fsxComponents) {
		b2Body* body = x.value.pBody;
		for (b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext()) {

			b2RayCastOutput output;
			if (!f->RayCast(&output, input, 0))
				continue;
			// TODO: This SHOULD filter out fixtues that don't collide with bullets... But doesn't seem to do so
			if(!(f->GetFilterData().maskBits & BULLET))
				continue;
			if (output.fraction < closestFraction && output.fraction > delta) {
				closestFraction = output.fraction;
				intersectionNormal = output.normal;
				eid = x.key;
			}
		}
		
	}
	return std::make_tuple( eid, intersectionNormal, closestFraction * bulletRange);

}

inline EntityView LevelScene::fireBullet(Vec2 position, odin::Direction8Way direction)
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
	switch (direction) {
	case odin::NORTH_WEST:
		collisionData = resolveBulletCollision(position, { -1,1 });
		length = std::get<2>(collisionData);
		rotation = -PI/4;
		offset = { SIN45 * -length/2, SIN45 * length/2 };
		break;
	case odin::NORTH_EAST:
		collisionData = resolveBulletCollision(position, { 1,1 });
		length = std::get<2>(collisionData);
		rotation = PI / 4;
		offset = { SIN45 * length / 2, SIN45 * length / 2 };
		break;
	case odin::SOUTH_WEST:
		collisionData = resolveBulletCollision(position, { -1,-1 });
		length = std::get<2>(collisionData);
		rotation = PI / 4;
		offset = { SIN45 * -length / 2, SIN45 * -length / 2 };
		break;
	case odin::SOUTH_EAST:
		collisionData = resolveBulletCollision(position, { 1,-1 });
		length = std::get<2>(collisionData);
		rotation = -PI / 4;
		offset = { SIN45 * length / 2, SIN45 * -length / 2 };
		break;
	case odin::NORTH:
		collisionData = resolveBulletCollision(position, { 0,1 });
		length = std::get<2>(collisionData);
		rotation = PI / 2;
		offset = { 0, length / 2 };
		break;
	case odin::SOUTH:
		collisionData = resolveBulletCollision(position, { 0, -1 });
		length = std::get<2>(collisionData);
		rotation = -PI / 2;
		offset = { 0, -length / 2 };
		break;
	case odin::WEST:
		collisionData = resolveBulletCollision(position, { -1,0 });
		length = std::get<2>(collisionData);
		offset = { -length / 2, 0 };
		break;
	case odin::EAST:
		collisionData = resolveBulletCollision(position, { 1,0 });
		length = std::get<2>(collisionData);
		offset = { length / 2 + 20, 0 };
		break;
	default:
		break;
	}

	EntityId eid("bullet", _bulletCount++);

	if (!entities.add(eid, Entity(position+offset, rotation)))
		std::cout << "Entity " << eid << " already exists.\n";

	auto bGfx = gfxComponents.add(eid, GraphicalComponent::makeRect(length, 8.0f, { 255.f, 255.f, 255.f }));
	bGfx->texture = BULLET_TEXTURE;
	if (!bGfx)
		std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

	if (!animComponents.add(eid, AnimatorComponent({ 8 }, odin::FADEOUT)))
		std::cout << "Entity " << eid << " already has an AnimationComponent.\n";

 	camera.shake();

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