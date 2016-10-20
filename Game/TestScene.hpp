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

	float _scale;

	TestScene( int width, int height, float scale )
		: LevelScene( width, height, "Audio/Banks/MasterBank" )
        , factory( EntityFactory::instance() )
		, _scale( scale )
	{
	}

	unsigned short _bulletCount = 0;

	void init( unsigned ticks )
    {
        LevelScene::init( ticks );

		auto background = gfxComponents.add(
			EntityId(0), GraphicalComponent::makeRect( width / _scale, height / _scale));
		background->texture = 4;

        listeners.add( "quit", [this]( const InputManager& inmn, EntityId eid ) {
            if ( inmn.wasKeyPressed( SDLK_BACKSPACE ) )
                this->expired = true;
        } );

		factory->makePlayer(this, "player");

		factory->makePlatform(this, "plat1", 10);

		//factory->makeRect(this, "box", { 1,1 }, { 1,1 }, 0, { 1,1,1 });

		//fireBullet({ -170, 5.5f }, { 100, 0 });

		//GLuint nul = load_texture( "null.png", 0 );

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

		if (!fsxComponents.add(eid, PhysicalComponent::makeCircle(.05f, b2world, bodyDef, 0.01f)))
			std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";

		return EntityView(eid, this);
	}
};
