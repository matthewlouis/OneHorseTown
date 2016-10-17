#pragma once

#include <Odin/SceneManager.hpp>
#include <Odin/TextureManager.hpp>

using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::InputManager;
using odin::Entity;
using odin::EntityId;
using odin::EntityView;
using odin::Scene;

class TestScene : public Scene{
public:

	int _height, _width;
	float _scale;

	TestScene(int height, int width, float scale, GLuint program, SDL_Renderer *renderer)
		:Scene(program, renderer, "Audio/Banks/MasterBank")
		, _height(height)
		, _width(width)
		, _scale(scale)
	{}

	unsigned short _bulletCount = 0;

	virtual void setup_scene() {

		auto background = gfxComponents.add(
			EntityId(0), GraphicalComponent::makeRect(_width / _scale, _height / _scale));
		background->texture = 4;

	    int pAnimInfo[3] = { 1, 10, 3 }; //idle 1 frame, run 10 frame, jump 3 frame
		auto pGfx = gfxComponents.add("player", GraphicalComponent::makeRect(2, 2, glm::vec3(1,1,1), 1.0, true, 3, pAnimInfo));
		pGfx->texture = 3;

		b2BodyDef playerDef;
		playerDef.position = { -7, -4 };
		playerDef.fixedRotation = true;
		playerDef.type = b2_dynamicBody;
		playerDef.gravityScale = 2;

		auto pFsx = fsxComponents.add("player", PhysicalComponent::makeRect(2, 2, b2world, playerDef));

		listeners.add("player", [&](const InputManager& inmn, EntityId eid) {
			return player_input(inmn, EntityView(eid, this));
		});

		addEqTri({ "tri", 0 }, 2, { 5, -3 }, 0, { 1, 0, 0 }, b2_kinematicBody);
		auto whttri = addEqTri({ "tri", 1 }, 2, { 0, 0 }, 0, { 1, 1, 1 }, b2_dynamicBody);
		addEqTri({ "tri", 2 }, 2, { 0.5, 2.5 }, 0, { 0, 0, 1 }, b2_dynamicBody);
		addEqTri({ "tri", 3 }, 2, { 7, -1 }, 0, { 0, 1, 0 }, b2_kinematicBody);

		fsxComponents[{"tri", 0}]->SetAngularVelocity(-1);
		fsxComponents[{"tri", 3}]->SetAngularVelocity(2);

		b2RevoluteJointDef rjd;
		b2BodyDef bodyDef;
		auto ground = b2world.CreateBody(&bodyDef);
		rjd.Initialize(ground, fsxComponents[{"tri", 1}].pBody, { 0, 0 });
		b2world.CreateJoint(&rjd);

		//             eid    dimens   pos   rot   color       body_type
		addRect("rect", { 1, 2 }, { 0, 7 }, 0, { 1, 1, 0 }, b2_dynamicBody);
		addRect({ "box", 0 }, { 1, 1 }, { -.5, 4 }, 0, { 1, 0, 1 }, b2_dynamicBody);
		auto pnkcrate = addRect({ "box", 1 }, { 1, 1 }, { -.5, 5 }, 0, { 1, 0, 1 }, b2_dynamicBody);
		addRect({ "box", 2 }, { 1, 1 }, { -.5, 6 }, 0, { 1, 0, 1 }, b2_dynamicBody);
		addRect({ "box", 3 }, { 1, 1 }, { -.5, 7 }, 0, { 1, 0, 1 }, b2_dynamicBody);
		addRect({ "box", 4 }, { 1, 1 }, { -.5, 8 }, 0, { 1, 0, 1 }, b2_dynamicBody);
		auto blucrate = addRect({ "box", 5 }, { 1, 1 }, { .1f, 90 }, 0, { 0, 1, 1 }, b2_dynamicBody);
		addRightTri({ "rtri", 0 }, { -2, 2 }, { 0, -3 }, 0, { 0, 1, 1 }, b2_dynamicBody);
		auto ylwramp = addRightTri({ "rtri", 1 }, { 3, 1 }, { -2, -3 }, 0, { 1, 1, 0 });

		fireBullet({ -170, 5.5f }, { 100, 0 });

		//GLuint nul = load_texture( "null.png", 0 );
		GLuint nul = odin::load_texture< GLubyte[4] >(0, 1, 1, { 0xFF, 0xFF, 0xFF, 0xFF });
		GLuint tex = odin::load_texture(1, "Textures/crate.png");
		GLuint tex2 = odin::load_texture(2, "Textures/crate2.png");
		GLuint tex3 = odin::load_texture(3, "Textures/CowboySS.png");
		GLuint tex4 = odin::load_texture(4, "Textures/background.png");

		blucrate.gfxComponent()->texture = 2;
		pnkcrate.gfxComponent()->texture = 1;
		ylwramp.gfxComponent()->texture = 1;
		whttri.gfxComponent()->texture = 1;

		b2BodyDef floorDef;
		b2EdgeShape floorShape;
		floorShape.Set({ -10, -8 }, { 10, -8 });

		fsxComponents["floor"] = b2world.CreateBody(&floorDef);
		fsxComponents["floor"]->CreateFixture(&floorShape, 1)
			->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);

		floorShape.Set({ 10, +8 }, { 10, -8 });

		fsxComponents["wall"] = b2world.CreateBody(&floorDef);
		fsxComponents["wall"]->CreateFixture(&floorShape, 1)
			->SetFriction(odin::PhysicalComponent::DEFAULT_FRICTION);

		//load common events and play music
		audioEngine.loadEvent("event:/Music/EnergeticTheme");
		audioEngine.loadEvent("event:/Desperado/Shoot");

		audioEngine.playEvent("event:/Music/EnergeticTheme");
	}

	EntityView fireBullet(Vec2 position, Vec2 velocity)
	{
		EntityId eid("bullet", _bulletCount++);

		if (!entities.add(eid, Entity(position, 0)))
			std::cout << "Entity " << eid << " already exists.\n";

		if (!gfxComponents.add(eid, GraphicalComponent::makeRect(3.f, .1f)))
			std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

		b2BodyDef bodyDef;
		bodyDef.position = position;
		bodyDef.linearVelocity = velocity;
		bodyDef.type = b2_dynamicBody;
		bodyDef.bullet = true;

		if (!fsxComponents.add(eid, PhysicalComponent::makeCircle(.05f, b2world, bodyDef, 0.01)))
			std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";

		return EntityView(eid, this);
	}

	template< int PLAYER = 0 >
	void player_input(const InputManager& mngr, EntityView ntt)
	{
		static_assert(PLAYER >= 0 && PLAYER < odin::ControllerManager::MAX_PLAYERS, "");

		b2Body& body = *ntt.fsxComponent()->pBody;
		GraphicalComponent& gfx = *ntt.gfxComponent();

		Vec2 vel = body.GetLinearVelocity();
		float maxSpeed = 5.5f;
		float actionLeft = mngr.isKeyDown(SDLK_LEFT) ? 1 : 0;
		float actionRight = mngr.isKeyDown(SDLK_RIGHT) ? 1 : 0;
		int actionDir = 0;

		//adjust facing direction
		if (actionLeft)
			gfx.direction = odin::LEFT;
		if (actionRight)
			gfx.direction = odin::RIGHT;

		//b2Fixture* pFixt = body.GetFixtureList();

<<<<<<< HEAD
		
=======
		if (actionLeft == 0 && actionRight == 0)
		{
			//pFixt->SetFriction( 2 );
			vel.x = tween<float>(vel.x, 0, 12 * (1 / 60.0));
			gfx.switchAnimState(0); //idle state
		}
		else
		{
			//pFixt->SetFriction( 0 );
			vel.x -= actionLeft * (20 + 1) * (1 / 60.0);
			vel.x += actionRight * (20 + 1) * (1 / 60.0);
			vel.x = glm::clamp(vel.x, -maxSpeed, +maxSpeed);
			gfx.switchAnimState(1); //running
		}
>>>>>>> develop

		if (mngr.wasKeyPressed(SDLK_UP)) {
			vel.y = 11;
		}

		if (mngr.wasKeyReleased(SDLK_UP) && vel.y > 0) {
			vel.y *= 0.6f;
		}

		// Handle Jump input on button A
		if (mngr.gamepads.wasButtonPressed(0, SDL_CONTROLLER_BUTTON_A))
			vel.y = 11;

		if (mngr.gamepads.wasButtonReleased(0, SDL_CONTROLLER_BUTTON_A) && vel.y > 0)
			vel.y *= 0.6f;

<<<<<<< HEAD
		// Handle Duck input on button B
		if (mngr.gamepads.wasButtonPressed(0, SDL_CONTROLLER_BUTTON_X))
		{

		}
		if (mngr.gamepads.wasButtonReleased(0, SDL_CONTROLLER_BUTTON_X))
		{

		}

		// Handle Shoot input on button B
		if (mngr.gamepads.wasButtonPressed(0, SDL_CONTROLLER_BUTTON_B))
		{
			fireBullet(Vec2(0, 0), Vec2(1, 1));
		}
		if (mngr.gamepads.wasButtonReleased(0, SDL_CONTROLLER_BUTTON_B))
		{

		}

		// Handle directions from left joystick axis
		//if (mngr.gamepads.joystickAxisX(0) != 0)
		{
			actionDir = mngr.gamepads.joystickAxisX(0);
			//printf("actionDir: %d ", actionDir);
			printf("joystickX: %d, joystickY: %d\n", mngr.gamepads.joystickAxisX(0), mngr.gamepads.joystickAxisY(0));
		}
		if (mngr.gamepads.joystickAxisY(0) != 0)
		{
			
		}

		// some funtionality to bring up menu or exit scene/game
		if (mngr.wasKeyPressed(SDLK_ESCAPE))
		{
			
		}

		if (actionLeft == 0 && actionRight == 0 && actionDir == 0)
		{
			//pFixt->SetFriction( 2 );
			vel.x = tween<float>(vel.x, 0, 12 * (1 / 60.0));
		}
		else
		{
			//pFixt->SetFriction( 0 );
			vel.x += (float)actionDir * (20 + 1) * (1 / 60.0); // for use w/gamepad
			vel.x -= actionLeft * (20 + 1) * (1 / 60.0); // for use w/keyboard
			vel.x += actionRight * (20 + 1) * (1 / 60.0); // for use w/keyboard
			vel.x = glm::clamp(vel.x, -maxSpeed, +maxSpeed);
		}
=======
		//for testing audio
		if (mngr.wasKeyPressed(SDLK_SPACE))
			audioEngine.playEvent("event:/Desperado/Shoot"); //simulate audio shoot
		if (mngr.wasKeyPressed(SDLK_1))
			audioEngine.setEventParameter("event:/Music/EnergeticTheme", "Energy", 0.0); //low energy test
		if (mngr.wasKeyPressed(SDLK_2))
			audioEngine.setEventParameter("event:/Music/EnergeticTheme", "Energy", 1.0); //high energy test
>>>>>>> develop

		body.SetLinearVelocity(vel);
	}
};
