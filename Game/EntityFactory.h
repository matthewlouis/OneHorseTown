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
	PLAYER1_TEXTURE,
	PLAYER2_TEXTURE,
	PLAYER3_TEXTURE,
	PLAYER4_TEXTURE,
	ARM_TEXTURE,
	CRATE1,
	CRATE2,
	GROUND1,
	BARREL,
	BACKGROUND,
	TITLE,
	PRESS_BUTTON,
	BULLET_TEXTURE,
	INTRO,
	BLACK,
	WHITE,
	RED,
	WIN_TEXTURE,
	READY_TEXTURE,
	BACKGROUND_ANIM,
    PLAYER_0_CARD,
    PLAYER_1_CARD,
    PLAYER_2_CARD,
    PLAYER_3_CARD,
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
	PLAYER_ARM = 1 << 4,
    PARTICLE = 1 << 5,
	DEAD_ENTITY = 1 << 6
};




namespace odin
{

	inline namespace factory
	{
		extern const int PLAYER_TAG;

		// "using namespace odin::factory;" to just use these functions.

		// graphics component:physics component scale 10:1
		static const int physicsScale = 10;

		// used for specifying dimensions of objects. 
		static struct Dim {
			float x; // actual scale for graphics comp
			float y;
			Dim getPhysicsDim() { return Dim{ x / physicsScale, y / physicsScale }; } // return physics dim to scale
		};
		// object dimensions
		static Dim playerDim{ 32,32 }; // intially 32x32
		static Dim horse{ 44,36 }; // initially 44x36
		static Dim platform{ 16,16 }; // initially 16x16

		template< typename T >
		void make_player(T* pScene, EntityId eid, Vec2 pos, uint16_t playerNum)
		{
			pScene->entities[eid];
			pScene->entities[{ "playes", playerNum }];

			decltype(auto) ntt = pScene->entities[eid];
			ntt.position = pos;
			ntt.setBase(typename T::EntityPlayerType{ playerNum, pScene });

			ntt.pDrawable = pScene->newGraphics(GraphicalComponent::makeRect(playerDim.x, playerDim.y));

			//assign different color spritesheet for each player
			switch (playerNum) {
			case 0:
				ntt.pDrawable->texture = PLAYER1_TEXTURE;
				break;
			case 1:
				ntt.pDrawable->texture = PLAYER2_TEXTURE;
				break;
			case 2:
				ntt.pDrawable->texture = PLAYER3_TEXTURE;
				break;
			case 3:
				ntt.pDrawable->texture = PLAYER4_TEXTURE;
				break;
			}

			decltype(auto) nttarm = pScene->entities[{ "playes", playerNum }];

			//add arm
			nttarm.pDrawable = pScene->newGraphics(GraphicalComponent::makeRect(playerDim.x, playerDim.y));
			nttarm.pDrawable->texture = ARM_TEXTURE;
			nttarm.pDrawable->visible = false;
			//idle 10 frame, run 10 frame, jump 3 frame, shoot 3 frame (one armed, one armless)	
//get_components< AnimatorComponent >( pScene ).add( eid, { 10, 10, 3, 3, 10, 10, 3, 3 } );

			ntt.pAnimator = pScene->newAnimator({ 10, 10, 3, 3, 10, 10, 3, 3, 2, 10 });

			//Set up arm
																//5 x shoot animation at 4 frames per anim
			nttarm.pAnimator = pScene->newAnimator({ 4, 4, 4, 4, 4 });
			nttarm.pAnimator->play = false;
			nttarm.pAnimator->loop = false;

			b2BodyDef playerDef;
			playerDef.position = pos;
			playerDef.fixedRotation = true;
			playerDef.type = b2_dynamicBody;
			playerDef.gravityScale = 2;
			playerDef.userData = ntt.base();

			// get player rec
			auto pRec = PhysicalComponent::makeRect(playerDim.getPhysicsDim().x / 2, playerDim.getPhysicsDim().y, pScene->b2world, playerDef, 1.0, PLAYER, PLATFORM | PLAYER | BULLET);
			
			// add player foot sensor fixture
			//shape definition for sensonr fixture
			b2PolygonShape polygonShape;
			polygonShape.SetAsBox(1, 2); //a 2x4 rectangle
			//fixture definition
			//b2FixtureDef myFixtureDef;
			//myFixtureDef.shape = &polygonShape;
			//myFixtureDef.density = 1;
			//polygonShape.SetAsBox(0.3, 0.3, b2Vec2(0, -2), 0);
			//myFixtureDef.isSensor = true;
			//b2Fixture* footSensorFixture = pRec->CreateFixture(&myFixtureDef);
			//footSensorFixture->SetUserData((void*)3);
			
			// assign player phys to entity
			ntt.pBody = pRec.pBody;
			pRec.pBody = nullptr;

			//a,rm
			b2BodyDef armDef;
			armDef.position = pos;
			armDef.type = b2_staticBody;

			//player arm physics component
			auto paFsx = PhysicalComponent::makeRect(playerDim.getPhysicsDim().x, playerDim.getPhysicsDim().y, pScene->b2world, armDef, 1.0, PLAYER_ARM, 0);
			nttarm.pBody = paFsx.pBody;
			paFsx.pBody = nullptr;
		}

		template< typename T >
		void make_horse(T* pScene, EntityId eid, Vec2 pos)
		{
			decltype(auto) ntt = pScene->entities[eid];


			ntt.pDrawable = pScene->newGraphics(GraphicalComponent::makeRect(horse.x, horse.y));
			ntt.pDrawable->texture = HORSE_TEXTURE;

			b2BodyDef horseDef;
			horseDef.position = pos;
			horseDef.fixedRotation = true;
			horseDef.type = b2_dynamicBody;
			horseDef.gravityScale = 2;

			auto hFsx = PhysicalComponent::makeRect(horse.getPhysicsDim().x, horse.getPhysicsDim().y, pScene->b2world, horseDef, 1.0, HORSE, PLATFORM);
			ntt.pBody = hFsx.pBody;
			hFsx.pBody = nullptr;
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


			Vec2 pos{ offset.x,offset.y };
			x = 1;
			EntityId eid;
			uint16_t physID = 0;
			uint16 i;
			for (i = 0; i < length; i++)
			{
				eid = { id, i };
				decltype(auto) ntt = pScene->entities[eid];


				pos.x += platform.x / 2;

				ntt.position = pos;
				ntt.pDrawable = pScene->newGraphics(
					GraphicalComponent::makeRect(platform.x, platform.y, { 1,1,1 }));

				ntt.pDrawable->texture = text;
				//makeRect(scene, { id, i }, { 1, 1 }, pos, 0, { 1,1,1 }, GROUND1);

				pos.x += platform.x / 2;
				physID++;
			}

			// define another entity for the floors physics only
			eid = { id, physID };
			decltype(auto) ntt = pScene->entities[eid];
			ntt.position = { pos.x + (platform.x*length),pos.y };

			b2BodyDef bodyDef;
			bodyDef.position = { offset.x / physicsScale + (platform.getPhysicsDim().x*length) / 2 ,offset.y / physicsScale - 0.1f};
			bodyDef.angle = 0;
			bodyDef.type = b2_staticBody;

			auto pb = PhysicalComponent::makeRect(platform.getPhysicsDim().x*length - 1, platform.getPhysicsDim().y + 0.1, pScene->b2world, bodyDef, 1.0, PLATFORM,  PLAYER | BULLET | DEAD_ENTITY);
			ntt.pBody = pb.pBody;
			pb.pBody = nullptr;
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
		//odin::load_texture< GLubyte[4] >(NULL_TEXTURE, 1, 1, { 0xFF, 0xFF, 0xFF, 0xFF });
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

