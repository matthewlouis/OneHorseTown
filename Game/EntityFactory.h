#pragma once

#include <Odin/Scene.h>
#include <Odin/TextureManager.hpp>
#include "Scenes.hpp"

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
	BACKGROUND
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

        get_components< AnimatorComponent >( pScene ).add( eid, { 1, 10, 3 } );

        b2BodyDef playerDef;
        playerDef.position = pos;
        playerDef.fixedRotation = true;
        playerDef.type = b2_dynamicBody;
        playerDef.gravityScale = 2;

        auto pFsx = get_components< PhysicalComponent >( pScene ).add( eid,
            PhysicalComponent::makeRect( 1.6, 3.2, pScene->b2world, playerDef ) );
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
	
private:

	// Load all of the textures
	EntityFactory() {
		odin::load_texture< GLubyte[4] >(NULL_TEXTURE, 1, 1, { 0xFF, 0xFF, 0xFF, 0xFF });
		odin::load_texture(CRATE1, "Textures/crate.png");
		odin::load_texture(CRATE2, "Textures/crate2.png");
		odin::load_texture(GROUND1, "Textures/ground.png");
		odin::load_texture(GROUND2, "Textures/ground2.png");
		odin::load_texture(PLAYER_TEXTURE, "Textures/CowboySS.png");
		odin::load_texture(BACKGROUND, "Textures/background.png");
		odin::load_texture(HORSE_TEXTURE, "Textures/horse_dense.png");
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

