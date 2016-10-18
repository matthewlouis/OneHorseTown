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

	void makePlayer(odin::EntityId id, float width, float height, odin::Scene* scene);

private:

	// Load all of the textures
	EntityFactory() {
		odin::load_texture< GLubyte[4] >(NULL_TEXTURE, 1, 1, { 0xFF, 0xFF, 0xFF, 0xFF });
		odin::load_texture(CRATE1, "Textures/crate.png");
		odin::load_texture(CRATE2, "Textures/crate2.png");
		odin::load_texture(PLAYER, "Textures/CowboySS.png.png");
		odin::load_texture(BACKGROUND, "Textures/background.png");
	}
};
