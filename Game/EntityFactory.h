#pragma once

#include <Odin/Scene.h>
#include <Odin/TextureManager.hpp>

enum Textures {
	NULL_TEXTURE,
	PLAYER,
	CRATE1,
	CRATE2,
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

using odin::Entity;
using odin::EntityId;
using odin::EntityView;
using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::InputManager;
using odin::Scene;

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
		Scene*	   scene,
		EntityId   eid, 
		Vec2	   size = { 1, 2 }
	);
	
	EntityView makeRightTri(
		Scene*	   scene,
		EntityId   eid, 
		Vec2       dimen,
		Vec2       pos,
		float      rot,
		glm::vec3  color,
		Textures   text = CRATE1,
		b2BodyType bodyType = b2_dynamicBody
	);
	

	EntityView makeEqTri(
		Scene*	   scene,
		EntityId   eid,
		float      length,
		Vec2       pos,
		float      rot,
		glm::vec3  color,
		Textures   text = CRATE1,
		b2BodyType bodyType = b2_dynamicBody
	);

	EntityView makeRect(
		Scene*	   scene,
		EntityId   eid,
		Vec2       dimen,
		Vec2       pos,
		float      rot,
		glm::vec3  color,
		Textures   text = CRATE1,
		b2BodyType bodyType = b2_staticBody
	);

	void makePlatform(
		Scene*		scene,
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
		odin::load_texture(PLAYER, "Textures/CowboySS.png");
		odin::load_texture(BACKGROUND, "Textures/background.png");
	}
};
