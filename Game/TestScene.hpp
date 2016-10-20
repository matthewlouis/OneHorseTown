#pragma once

#include <Odin/SceneManager.hpp>
#include <Odin/TextureManager.hpp>
#include "EntityFactory.h"

using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::InputManager;
using odin::Entity;
using odin::EntityId;
using odin::EntityView;
using odin::Scene;

class TestScene : public Scene{
public:

	EntityFactory* factory;

	int _height, _width;
	float _scale;
	
	//for simulating energy - alpha presentation
	float energyLevel = 0;

	TestScene(int height, int width, float scale, GLuint program, SDL_Renderer *renderer)
		:Scene(program, renderer, "Audio/Banks/MasterBank")
		, _height(height)
		, _width(width)
		, _scale(scale)
	{
		factory = EntityFactory::instance();
	}

	unsigned short _bulletCount = 0;

	virtual void setup_scene() {

		/*
		auto background = gfxComponents.add(
			EntityId(0), GraphicalComponent::makeRect(_width / _scale, _height / _scale));
		background->texture = BACKGROUND;
		
		float h = _height / _scale;
		float w = _width / _scale;
		*/

		auto bg = gfxComponents.add(EntityId(0), GraphicalComponent::makeRect(25, 16.5));
		bg->texture = BACKGROUND;

		factory->makePlayer(this, "player");

		factory->makeHorse(this, "horse");

		factory->makePlatform(this, "plat1", 3, {0, -3}); // Lower Middle
		factory->makePlatform(this, "plat2", 6, { 0.5, 3 }); // Upper middle
		
		factory->makePlatform(this, "plat3", 4, { -9, 5 }); // Top Left
		factory->makePlatform(this, "plat4", 4, { 10, 5 }); // Top Right
		
		factory->makePlatform(this, "plat5", 5, { -6, 0 }); // Middle Left
		factory->makePlatform(this, "plat6", 5, { 6, 0 }); // Middle Right
		
		factory->makePlatform(this, "plat7", 2, { -3, -5.5 }); // Lower Left
		factory->makePlatform(this, "plat8", 2, { 3, -5.5 }); // Lower Right
		

		//factory->makeRect(this, "box", { 1,1 }, { 1,1 }, 0, { 1,1,1 });

		//fireBullet({ -170, 5.5f }, { 100, 0 });

		//GLuint nul = load_texture( "null.png", 0 );

		b2BodyDef floorDef;
		b2EdgeShape floorShape;
		floorShape.Set({ -11, -8 }, { 11, -8 });

		fsxComponents["floor"] = b2world.CreateBody(&floorDef);
		fsxComponents["floor"]->CreateFixture(&floorShape, 1)
			->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);

		floorShape.Set({ 11, +10 }, { 11, -8 });

		fsxComponents["wallR"] = b2world.CreateBody(&floorDef);
		fsxComponents["wallR"]->CreateFixture(&floorShape, 1)
			->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);

		floorShape.Set({ -11, +10 }, { -11, -8 });

		fsxComponents["wallL"] = b2world.CreateBody(&floorDef);
		fsxComponents["wallL"]->CreateFixture(&floorShape, 1)
			->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);

		//load common events and play music
		audioEngine.loadEvent("event:/Music/EnergeticTheme");
		audioEngine.loadEvent("event:/Desperado/Shoot");

		audioEngine.playEvent("event:/Music/EnergeticTheme");
	}

	// Using bullet start position, the velocity  direction, and default facing direction.
	EntityView fireBullet(Vec2 position, Vec2 velocity, odin::FacingDirection direction)
	{
		//for alpha presentation, to simulate energy levels
		//more shots fired == more energy!
		if (energyLevel >= 1.0) {
			energyLevel = 1.0f;
		}
		else {
			energyLevel += 0.2f;
		}
		audioEngine.setEventParameter("event:/Music/EnergeticTheme", "Energy", energyLevel);
		audioEngine.playEvent("event:/Desperado/Shoot");

		double bulletOffset = 0.5;
		float bulletVelocity = 100;

		// first set facing direction offset for bullet, eventually bullet should have a odin::FacingDirection
		// correct bullet firing from left side using offset
		if (direction == odin::LEFT)
			position.x -= bulletOffset;

		// ensure a default case for 0 velocity incase joystick is neither held left or right.
		if (velocity.x == 0 && velocity.y == 0 && direction == odin::LEFT)
			velocity.x = -1;

		if (velocity.x == 0 && velocity.y == 0 && direction == odin::RIGHT)
		{
			velocity.x = 1;
		}

		// get new velocity based on direction and bullet velocity
		velocity.x *= bulletVelocity;
		velocity.y *= bulletVelocity;

		EntityId eid("bullet", _bulletCount++);

		if (!entities.add(eid, Entity(position, 0)))
			std::cout << "Entity " << eid << " already exists.\n";

		if (!gfxComponents.add(eid, GraphicalComponent::makeRect(.5f, .1f)))
			std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

		b2BodyDef bodyDef;
		bodyDef.position = position;
		bodyDef.linearVelocity = velocity;
		bodyDef.type = b2_dynamicBody;
		bodyDef.bullet = true;

		if (!fsxComponents.add(eid, PhysicalComponent::makeCircle(.05f, b2world, bodyDef, 0.01f)))
			std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";

		return EntityView(eid, this);
	}

	//template< int PLAYER = 0 >
	virtual void player_input(const InputManager& mngr, EntityView ntt)
	{
		constexpr int PLAYER = 0;
		static_assert(PLAYER >= 0 && PLAYER < odin::ControllerManager::MAX_PLAYERS, "");


		b2Body& body = *ntt.fsxComponent()->pBody;
		GraphicalComponent& gfx = *ntt.gfxComponent();

		Vec2 vel = body.GetLinearVelocity();
		float maxSpeed = 5.5f;
		float actionLeft = mngr.isKeyDown(SDLK_LEFT) ? 1 : 0;
		float actionRight = mngr.isKeyDown(SDLK_RIGHT) ? 1 : 0;
		int actionDir = 0;
		Vec2 aimDir(0, 0);


		//adjust facing direction
		if (actionLeft)
			gfx.direction = odin::LEFT;
		if (actionRight)
			gfx.direction = odin::RIGHT;

		//b2Fixture* pFixt = body.GetFixtureList();

		if (mngr.wasKeyPressed(SDLK_UP)) {
			vel.y = 12;
		}

		if (mngr.wasKeyReleased(SDLK_UP) && vel.y > 0) {
			vel.y *= 0.6f;
		}

		// Handle directions from left joystick axis
		actionDir = mngr.gamepads.joystickAxisX(PLAYER);
		aimDir.x = mngr.gamepads.joystickDir(PLAYER).x; //50 is currently bullet fire velocity. 
		aimDir.y = -mngr.gamepads.joystickDir(PLAYER).y;

		//adjust facing direction for joystick
		if (actionDir == -1)
			gfx.direction = odin::LEFT;
		if (actionDir == 1)
			gfx.direction = odin::RIGHT;

		// Handle Jump input on button A
		if (mngr.gamepads.wasButtonPressed(PLAYER, SDL_CONTROLLER_BUTTON_A))
			vel.y = 12;

		if (mngr.gamepads.wasButtonReleased(PLAYER, SDL_CONTROLLER_BUTTON_A) && vel.y > 0)
			vel.y *= 0.6f;

		// Handle Duck input on button X
		if (mngr.gamepads.wasButtonPressed(PLAYER, SDL_CONTROLLER_BUTTON_X))
		{

		}
		if (mngr.gamepads.wasButtonReleased(PLAYER, SDL_CONTROLLER_BUTTON_X))
		{

		}

		// Handle Shoot input on button B
		if (mngr.gamepads.wasButtonPressed(PLAYER, SDL_CONTROLLER_BUTTON_B))
		{
			fireBullet({body.GetPosition().x,body.GetPosition().y}, aimDir, gfx.direction);
		}
		if (mngr.gamepads.wasButtonReleased(PLAYER, SDL_CONTROLLER_BUTTON_B))
		{

		}

		// some funtionality to bring up menu or exit scene/game
		if (mngr.wasKeyPressed(SDLK_ESCAPE) || mngr.gamepads.wasButtonPressed(PLAYER, SDL_CONTROLLER_BUTTON_START))
		{
			
		}

		if (actionLeft == 0 && actionRight == 0 && actionDir == 0)
		{
			//pFixt->SetFriction( 2 );
			vel.x = tween<float>(vel.x, 0, 12 * (1 / 60.0));
			gfx.switchAnimState(0); //idle state
		}
		else
		{
			//pFixt->SetFriction( 0 );
			vel.x += (float)actionDir * (20 + 1) * (1 / 60.0); // for use w/gamepad
			vel.x -= actionLeft * (20 + 1) * (1 / 60.0); // for use w/keyboard
			vel.x += actionRight * (20 + 1) * (1 / 60.0); // for use w/keyboard
			vel.x = glm::clamp(vel.x, -maxSpeed, +maxSpeed);
			gfx.switchAnimState(1); //running
		}

		//for testing audio
		if (mngr.wasKeyPressed(SDLK_1)) { //reset energy level
			energyLevel = 0;
			audioEngine.setEventParameter("event:/Music/EnergeticTheme", "Energy", energyLevel); //low energy test
		}

		body.SetLinearVelocity(vel);
	}
};
