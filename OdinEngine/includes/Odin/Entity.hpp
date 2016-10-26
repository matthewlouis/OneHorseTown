// Andrew Meckling
#pragma once

#include "Math.hpp"
#include "EntityId.hpp"
#include "GraphicalComponent.hpp"
#include "PhysicalComponent.hpp"
#include "AnimatorComponent.hpp"
#include "InputManager.hpp"

namespace odin {

	enum class ComponentType
	{
		Graphical, Physical, Animator,
	};

	// Base entity object of the entity-component system. Contains 
	// data required by most components.
	struct Entity
	{
		Vec2        position = { 0, 0 };
		float       rotation = 0;
        float       texAdjust[ 4 ] = { 0, 0, 1, 1 };
		unsigned    flags = 0;

		Entity() = default;

		Entity( Vec2 pos, float rot, unsigned flags = 0 )
			: position( pos )
			, rotation( rot )
			, flags( flags )
		{
		}
	};
}