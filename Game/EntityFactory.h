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

// All the possible textures
enum Textures {
	NULL_TEXTURE,
	PLAYER_TEXTURE,
	ARM_TEXTURE,
	CRATE1,
	CRATE2,
	GROUND1,
	GROUND2,
	HORSE_TEXTURE,
	BACKGROUND,
	TITLE,
	PRESS_BUTTON,
	BULLET_TEXTURE
};

// Where the entity is positioned relative to
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

/*
 * Bit shifted enum used for collision detection in the physics engine.
 * Each entity should have a single category in this list (category bits), and a list of things it collides with using a bitwise AND (mask bits)
 * See PhysicalComponent factory methods or Box2D API for more details
 */
enum EntityTypes {
	PLAYER = 1 << 0,
	HORSE = 1 << 1,
	PLATFORM = 1 << 2,
	BULLET = 1 << 3,
	PLAYER_ARM = 1 << 4
};




namespace odin
{

inline namespace factory
{
    // "using namespace odin::factory;" to just use these functions.

	// graphics component:physics component scale 10:1
	static const int physicsScale = 10;

	// used for specifying dimensions of objects. 
	static struct Dim {
		float x; // actual scale for graphics comp
		float y;
		//Dim(float a, float b) { x = a / 2; y = b / 2; }
		Dim getPhysicsDim() { return Dim{ x / physicsScale, y / physicsScale }; } // return physics dim to scale
	};
	// object dimensions
	static Dim playerDim{ 21,21 }; // intially 32x32
	static Dim horse{29,24}; // initially 44x36
	static Dim platform{10,10}; // initially 16x16

    template< typename T >
    void make_player( T* pScene, EntityId eid, Vec2 pos, uint16_t playerNum )
    {
        auto pGfx = get_components< GraphicalComponent >( pScene ).add( eid,
            GraphicalComponent::makeRect( playerDim.x, playerDim.y ) );
        pGfx->texture = PLAYER;

		//add arm
		auto paGfx = get_components< GraphicalComponent >(pScene).add({ "playes", playerNum },
			GraphicalComponent::makeRect(playerDim.x, playerDim.y));
		paGfx->texture = ARM_TEXTURE;
		paGfx->visible = false;
												//idle 10 frame, run 10 frame, jump 3 frame, shoot 3 frame (one armed, one armless)	
        get_components< AnimatorComponent >( pScene ).add( eid, { 10, 10, 3, 3, 10, 10, 3, 3 } );
		
		//Set up arm
															//5 x shoot animation at 4 frames per anim
		auto armAnm = get_components< AnimatorComponent >(pScene).add({"playes", playerNum}, { 4, 4, 4, 4, 4 });
		armAnm->play = false;
		armAnm->loop = false;

        b2BodyDef playerDef;
        playerDef.position = pos;
        playerDef.fixedRotation = true;
        playerDef.type = b2_dynamicBody;
        playerDef.gravityScale = 2;

        auto pFsx = get_components< PhysicalComponent >( pScene ).add( eid,
            PhysicalComponent::makeRect( playerDim.getPhysicsDim().x/2, playerDim.getPhysicsDim().y, pScene->b2world, playerDef, 1.0, PLAYER, PLATFORM | PLAYER | BULLET ) );


		//a,rm
		b2BodyDef armDef;
		armDef.position = pos;
		armDef.type = b2_staticBody;

		//player arm physics component
		auto paFsx = get_components< PhysicalComponent >(pScene).add({ "playes", playerNum },
			PhysicalComponent::makeRect(playerDim.getPhysicsDim().x, playerDim.getPhysicsDim().y, pScene->b2world, armDef, 1.0, PLAYER_ARM, 0));
    }

    template< typename T >
    void make_horse( T* pScene, EntityId eid, Vec2 pos )
    {
        auto hGfx = get_components< GraphicalComponent >( pScene ).add( eid,
            GraphicalComponent::makeRect( horse.x, horse.y ) );
        hGfx->texture = HORSE_TEXTURE;

        b2BodyDef horseDef;
        horseDef.position = pos; 
        horseDef.fixedRotation = true;
        horseDef.type = b2_dynamicBody;
        horseDef.gravityScale = 2;

        auto hFsx = get_components< PhysicalComponent >( pScene ).add( eid,
            PhysicalComponent::makeRect( horse.getPhysicsDim().x, horse.getPhysicsDim().y, pScene->b2world, horseDef, 1.0, HORSE, PLATFORM ) );
    }

	template< typename T >
	void make_platform(T* pScene, const char(&id)[7], int length, Vec2 offset = { 0, 0 }, Textures text = GROUND1, Anchors anchor = CENTRE)
	{
		Vec2 start;

		float x = 0, y = 0;

		switch (anchor)
		{
		case CENTRE:
			x += length / 2.0;
			start = { x, 0 };
			break;
		default:
			break;
		}

		Vec2 pos = offset;
		x = 1;
		EntityId eid;
		uint16 i;
		for (i = 0; i < length; i++)
		{
			eid = { id, i };
			if (!pScene->entities.add(eid, Entity(pos, 0)))
				std::cout << "Entity " << eid << " already exists.\n";

			if (!pScene->gfxComponents.add(eid,
				GraphicalComponent::makeRect(platform.x, platform.y, { 1,1,1 })))
				std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

			EntityView ntt = EntityView(eid, pScene);
			ntt.gfxComponent()->texture = GROUND1;
			//makeRect(scene, { id, i }, { 1, 1 }, pos, 0, { 1,1,1 }, GROUND1);
			
			

			pos.x += physicsScale;
			//offset.x += physicsScale; 
		}

		//float colliderPos = length / 2.0;
		//offset.x += colliderPos;
		//fset.x += pos.x;
		float newPos = offset.x + (platform.x*length / 2.0f) - (platform.x/2);
		// Define the physical properties of the platform
		b2BodyDef bodyDef;
		bodyDef.position = { newPos / physicsScale, offset.y / physicsScale };
		bodyDef.angle = 0;
		bodyDef.type = b2_staticBody;
		if (!pScene->fsxComponents.add({ id, i },
			PhysicalComponent::makeRect(platform.getPhysicsDim().x*length, platform.getPhysicsDim().y, pScene->b2world, bodyDef, 1.0, PLATFORM, HORSE | PLAYER)))
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

