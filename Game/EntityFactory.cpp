#include "EntityFactory.h"

using odin::EntityId;
using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::InputManager;
using odin::Scene;

void EntityFactory::makePlayer(EntityId id, float width, float height, Scene* scene) {

	int pAnimInfo[3] = { 1, 10, 3 }; //idle 1 frame, run 10 frame, jump 3 frame
	auto pGfx = scene->gfxComponents.add(id, GraphicalComponent::makeRect(2, 2, glm::vec3(1, 1, 1), 1.0, true, 3, pAnimInfo));
	pGfx->texture = PLAYER;

	b2BodyDef playerDef;
	playerDef.position = { -7, -4 };
	playerDef.fixedRotation = true;
	playerDef.type = b2_dynamicBody;
	playerDef.gravityScale = 2;

	auto pFsx = scene->fsxComponents.add(id, PhysicalComponent::makeRect(width, height, scene->b2world, playerDef));

	scene->listeners.add(id, [scene](const InputManager& inmn, EntityId eid) {
		return scene->player_input(inmn, eid);
	});
	
	// scene->entities.add(id, );
}
