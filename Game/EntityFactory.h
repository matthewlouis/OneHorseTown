#pragma once

#include <Odin/Scene.h>
#include <Odin/TextureManager.hpp>
#include <Odin/Entity.hpp>

using odin::Entity;
using odin::EntityId;
using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::InputManager;
using odin::Scene;

enum Textures {
	NULL_TEXTURE,
	PLAYER_TEXTURE,
	CRATE1,
	CRATE2,
	GROUND1,
	GROUND2,
	HORSE_TEXTURE,
	BACKGROUND,
	TITLE,
	PRESS_BUTTON
};

enum Anchors {
	CENTRE,
	// TODO
	//TOPLEFT,
	//TOPRIGHT,
	//TOPMIDDLE,
	//BOTTOMLEFT,
	//BOTTOMRIGHT,
	//BOTTOMMIDDLE,

};


enum EntityTypes {
	PLAYER = 1 << 0,
	HORSE = 1 << 1,
	PLATFORM = 1 << 2,
	BULLET = 1 << 3
};

namespace odin
{
inline namespace factory
{
    // "using namespace odin::factory;" to just use these functions.

    template< typename T >
    void make_player( T* pScene, EntityId eid, Vec2 pos )
    {
        int pAnimInfo[3] = { 1, 10, 3 }; //idle 1 frame, run 10 frame, jump 3 frame

        auto pGfx = get_components< GraphicalComponent >( pScene ).add( eid,
            GraphicalComponent::makeRect( 32, 32 ) );
        pGfx->texture = PLAYER;

        get_components< AnimatorComponent >( pScene ).add( eid, { 1, 10, 3 });

        b2BodyDef playerDef;
        playerDef.position = pos;
        playerDef.fixedRotation = true;
        playerDef.type = b2_dynamicBody;
        playerDef.gravityScale = 2;

        auto pFsx = get_components< PhysicalComponent >( pScene ).add( eid,
            PhysicalComponent::makeRect( 1.6, 3.2, pScene->b2world, playerDef, 1.0, PLAYER, PLATFORM | PLAYER ) );
    }

    template< typename T >
    void make_horse( T* pScene, EntityId eid, Vec2 pos )
    {
        auto hGfx = get_components< GraphicalComponent >( pScene ).add( eid,
            GraphicalComponent::makeRect( 44, 36 ) );
        hGfx->texture = HORSE_TEXTURE;

        b2BodyDef horseDef;
        horseDef.position = pos;
        horseDef.fixedRotation = true;
        horseDef.type = b2_dynamicBody;
        horseDef.gravityScale = 2;

        auto hFsx = get_components< PhysicalComponent >( pScene ).add( eid,
            PhysicalComponent::makeRect( 4.4, 3.6, pScene->b2world, horseDef, 1.0, HORSE, PLATFORM ) );
    }

	template< typename T >
	void make_platform(T* pScene, const char(&id)[7], int length, Vec2 offset = { 0, 0 }, Textures text = GROUND1, Anchors anchor = CENTRE)
	{
		Vec2 start;

		float x = 0, y = 0;

		switch (anchor)
		{
		case CENTRE:
			x -= length / 2.0;
			start = { x, 0 };
			break;
		default:
			break;
}

		Vec2 pos = start + offset;

		EntityId eid;
		uint16 i;
		for (i = 0; i < length; i++)
		{
			eid = { id, i };
			if (!pScene->entities.add(eid, Entity(pos, 0)))
				std::cout << "Entity " << eid << " already exists.\n";

			if (!pScene->gfxComponents.add(eid,
				GraphicalComponent::makeRect(16, 16, { 1,1,1 })))
				std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

			EntityView ntt = EntityView(eid, pScene);
			ntt.gfxComponent()->texture = GROUND1;

			//makeRect(scene, { id, i }, { 1, 1 }, pos, 0, { 1,1,1 }, GROUND1);
			pos.x++;
		}
		float colliderPos = length / 2.0;
		offset.x += colliderPos;
		b2BodyDef bodyDef;
		bodyDef.position = start + offset;
		bodyDef.angle = 0;
		bodyDef.type = b2_staticBody;

		
		if (!pScene->fsxComponents.add({ id, i },
		PhysicalComponent::makeRect(length, 1, pScene->b2world, bodyDef, 1.0, PLATFORM, HORSE | PLAYER)))
		std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";
		
}
}
}


// Singleton class for easily creating game objects
class EntityFactory {
public:

	// Don't allow copying
	EntityFactory(EntityFactory const&) = delete;
	void operator=(EntityFactory const&) = delete;


	static EntityFactory* instance() {
		static EntityFactory instance;

		return &instance;
	}

	/*
	EntityView makePlayer(
		LevelScene*	scene,
		EntityId    eid,
		Vec2	    size = { 1, 2 }
	);
	
	EntityView makeRightTri(
        LevelScene*	scene,
		EntityId    eid, 
		Vec2        dimen,
		Vec2        pos,
		float       rot,
		glm::vec3   color,
		Textures    text = CRATE1,
		b2BodyType  bodyType = b2_dynamicBody
	);
	

	EntityView makeEqTri(
        LevelScene*	scene,
		EntityId    eid,
		float       length,
		Vec2        pos,
		float       rot,
		glm::vec3   color,
		Textures    text = CRATE1,
		b2BodyType  bodyType = b2_dynamicBody
	);

	EntityView makeRect(
        LevelScene*	scene,
		EntityId    eid,
		Vec2        dimen,
		Vec2        pos,
		float       rot,
		glm::vec3   color,
		Textures    text = CRATE1,
		b2BodyType  bodyType = b2_staticBody
	);

	EntityView makeHorse(
        LevelScene*	scene,
		EntityId    eid,
		Vec2	    size = { 2, 2 }
	);

	void makePlatform(
        LevelScene* scene,
		const char	(&id)[6],
		uint16		length,
		Vec2		offset = { 0, 0 },
		Anchors		anchor = CENTRE
	);
	*/
private:

	// Load all of the textures
	EntityFactory() {
		odin::load_texture< GLubyte[4] >(NULL_TEXTURE, 1, 1, { 0xFF, 0xFF, 0xFF, 0xFF });
	}
};

inline EntityTypes operator|(EntityTypes a, EntityTypes b)
{
	return static_cast<EntityTypes>(static_cast<int>(a) | static_cast<int>(b));
}

inline EntityTypes operator&(EntityTypes a, EntityTypes b)
{
	return static_cast<EntityTypes>(static_cast<int>(a) & static_cast<int>(b));
}

