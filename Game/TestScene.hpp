#pragma once

#include <Odin/SceneManager.hpp>
#include <Odin/TextureManager.hpp>

#include "EntityFactory.h"
#include "Scenes.hpp"

using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::InputManager;
using odin::Entity;
using odin::EntityId;
using odin::Scene;

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

	TestScene(int width, int height, float scale, int numberPlayers)
		: LevelScene(width, height, "Audio/Banks/MasterBank")
		, factory(EntityFactory::instance())
		, _scale(scale)
	{
		players = (EntityView*)malloc(sizeof(EntityView) * numberPlayers);
		player_arms = (EntityView*)malloc(sizeof(EntityView) * numberPlayers);
		this->numberPlayers = 1;//numberPlayers; UNCOMMENT THIS!
	}

	void init( unsigned ticks )
    {
		//set arm offsets so it renders in correct location
		armOffsets[0] = Vec2(0.5, 0.9);
		armOffsets[1] = Vec2(0.6, 0.75);
		armOffsets[2] = Vec2(0.75, 0.475);
		armOffsets[3] = Vec2(0.6, 0.275);
		armOffsets[4] = Vec2(0.25, 0.05);

        LevelScene::init( ticks );

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


		/* FOR TESTING ANIMATION STATES REMEMBER TO UNCOMMENT
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
		*/

		//factory->makeHorse(this, "horse");
        //odin::make_horse( this, "horse", {0.0f, 5.f} );


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
