#include "EntityFactory.h"


EntityView EntityFactory::makePlayer(
	Scene*	   scene,
	EntityId   eid,
	Vec2	   size)
{
	int pAnimInfo[3] = { 1, 10, 3 }; //idle 1 frame, run 10 frame, jump 3 frame
	auto pGfx = scene->gfxComponents.add(eid, GraphicalComponent::makeRect(2, 2, glm::vec3(1, 1, 1), 1.0, true, 3, pAnimInfo));
	pGfx->texture = PLAYER;

	b2BodyDef playerDef;
	playerDef.position = { -7, -4 };
	playerDef.fixedRotation = true;
	playerDef.type = b2_dynamicBody;
	playerDef.gravityScale = 2;

	auto pFsx = scene->fsxComponents.add(eid, PhysicalComponent::makeRect(size.x, size.y, scene->b2world, playerDef));


	scene->listeners.add(eid, [scene](const InputManager& inmn, EntityId eid) {
		return scene->player_input(inmn, odin::EntityView(eid, scene));
	});
	
	return EntityView(eid, scene);
}


EntityView EntityFactory::makeRightTri(
	Scene*	   scene,
	EntityId   eid, 
	Vec2       dimen,
	Vec2       pos,
	float      rot,
	glm::vec3  color,
	Textures   text,
	b2BodyType bodyType)
{
	if (!scene->entities.add(eid, Entity(pos, rot)))
		std::cout << "Entity " << eid << " already exists.\n";

	if (!scene->gfxComponents.add(eid,
		GraphicalComponent::makeRightTri(dimen.x, dimen.y, color)))
		std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

	b2BodyDef bodyDef;
	bodyDef.position = pos;
	bodyDef.angle = rot;
	bodyDef.type = bodyType;

	if (!scene->fsxComponents.add(eid,
		PhysicalComponent::makeRightTri(dimen.x, dimen.y, scene->b2world, bodyDef)))
		std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";

	EntityView ntt = EntityView(eid, scene);
	ntt.gfxComponent()->texture = text;

	return ntt;

}

EntityView EntityFactory::makeEqTri(
	Scene*	   scene,
	EntityId   eid, 
	float      length,
	Vec2       pos,
	float      rot,
	glm::vec3  color,
	Textures   text,
	b2BodyType bodyType)
{
	if (!scene->entities.add(eid, Entity(pos, rot)))
		std::cout << "Entity " << eid << " already exists.\n";

	if (!scene->gfxComponents.add(eid, GraphicalComponent::makeEqTri(length, color)))
		std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

	b2BodyDef bodyDef;
	bodyDef.position = pos;
	bodyDef.angle = rot;
	bodyDef.type = bodyType;

	if (!scene->fsxComponents.add(eid, PhysicalComponent::makeEqTri(length, scene->b2world, bodyDef)))
		std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";

	EntityView ntt = EntityView(eid, scene);
	ntt.gfxComponent()->texture = text;

	return ntt;
}

EntityView EntityFactory::makeRect(
	Scene*	   scene,
	EntityId   eid,
	Vec2       dimen,
	Vec2       pos,
	float      rot,
	glm::vec3  color,
	Textures   text,
	b2BodyType bodyType)
{
	if (!scene->entities.add(eid, Entity(pos, rot)))
		std::cout << "Entity " << eid << " already exists.\n";

	if (!scene->gfxComponents.add(eid,
		GraphicalComponent::makeRect(dimen.x, dimen.y, color)))
		std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

	b2BodyDef bodyDef;
	bodyDef.position = pos;
	bodyDef.angle = rot;
	bodyDef.type = bodyType;

	if (!scene->fsxComponents.add(eid,
		PhysicalComponent::makeRect(dimen.x, dimen.y, scene->b2world, bodyDef)))
		std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";

	EntityView ntt = EntityView(eid, scene);
	ntt.gfxComponent()->texture = text;
	
	return ntt;
}

void EntityFactory::makePlatform(
	Scene*		scene,
	const char (&id)[6],
	uint16		length,
	Vec2		offset,
	Anchors		anchor) 
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

	Vec2 pos = start;

	for (uint16 i = 0; i < length; i++)
	{
		makeRect(scene, { id, i }, { 1, 1 }, pos, 0, { 1,1,1 });
		pos.x++;
	}
}