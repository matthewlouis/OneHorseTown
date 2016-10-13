// Andrew Meckling
#pragma once

#include "Math.hpp"
#include "EntityId.hpp"
#include "GraphicalComponent.hpp"
#include "PhysicalComponent.hpp"
#include "InputManager.hpp"

namespace odin {

	enum class ComponentType
	{
		Graphical, Physical, Input
	};

	// Base entity object of the entity-component system. Contains 
	// data required by most components.
	struct Entity
	{
		Vec2        position = { 0, 0 };
		float       rotation = 0;
		unsigned    flags = 0;

		Entity() = default;

		Entity(Vec2 pos, float rot, unsigned flags = 0)
			: position(pos)
			, rotation(rot)
			, flags(flags)
		{
		}
	};
}