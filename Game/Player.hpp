#pragma once
#include <Odin\Entity.hpp>
#include <Odin\EntityId.hpp>
#include <Odin\GraphicalComponent.hpp>
#include <Odin\AnimatorComponent.hpp>
#include <Odin\PhysicalComponent.hpp>
#include <glm\glm.hpp>

using odin::Entity;
using odin::EntityId;
using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::AnimatorComponent;
using odin::Scene;

enum PlayerState {
	IDLE = 0,
	RUNNING,
	IN_AIR,
	IDLE_SHOOTING,
	RUNNING_SHOOTING = 5,
	IN_AIR_SHOOTING = 6,
	HIT,
	DEAD
};

class Player {
public:
	const float FALL_THRESHOLD = 0.1f;
	//arm
	GraphicalComponent* arm_gfx;
	AnimatorComponent* arm_anim;

	//player
	GraphicalComponent* gfx;
	AnimatorComponent* anim;
	PhysicalComponent* psx;

	PlayerState currentState;
	bool falling = false;

	bool active = false;

	void init(GraphicalComponent *gfx,
			  AnimatorComponent *anim,
			  GraphicalComponent *arm_gfx, 
			  AnimatorComponent *arm_anim,
			  PhysicalComponent *psx) {
		this->gfx = gfx;
		this->anim = anim;
		this->arm_gfx = arm_gfx;
		this->arm_anim = arm_anim;
		this->psx = psx;
		active = true;
	}


	std::string getStateName() {
		switch (currentState) {
		case IDLE:
			return "IDLE";
			break;
		case RUNNING:
			return "RUNNING";
			break;
		case IN_AIR:
			return "IN_AIR";
			break;
		default:
			return "UNKNOWN";
			break;
		}
	}

	void update() {
		if (!active)
			return;

		Vec2 vel = psx->pBody->GetLinearVelocity();

		switch (currentState) {
			case IDLE:
				if (vel.y > FALL_THRESHOLD || vel.y < -FALL_THRESHOLD) {
					anim->switchAnimState(IN_AIR);
					anim->loop = false;
					currentState = IN_AIR;
					std::cout << "\nState: " << getStateName() << std::endl;
				}
				else if (abs(vel.x) > FALL_THRESHOLD) {
					anim->switchAnimState(RUNNING);
					anim->loop = true;
					currentState = RUNNING;
					std::cout << "\nState: " << getStateName() << std::endl;
				}
				break;
			case RUNNING:
				if (vel.y > FALL_THRESHOLD || vel.y < -FALL_THRESHOLD) {
					anim->switchAnimState(IN_AIR);
					anim->loop = false;
					currentState = IN_AIR;
					std::cout << "\nState: " << getStateName() << std::endl;
				}
				else if (abs(vel.x) <= FALL_THRESHOLD) {
					anim->switchAnimState(IDLE);
					anim->loop = true;
					currentState = IDLE;
				}
				break;
			case IDLE_SHOOTING:
				break;
			case IN_AIR:
			case IN_AIR_SHOOTING:
				if (vel.y < -FALL_THRESHOLD)
					falling = true;

				if (falling && vel.y <= FALL_THRESHOLD && vel.y >= -FALL_THRESHOLD) {
					falling = false;
					
					if (abs(vel.x) < FALL_THRESHOLD) {
						anim->switchAnimState(IDLE);
						currentState = IDLE;
					}
					else {
						anim->switchAnimState(RUNNING);
						anim->currentFrame = 4;
						currentState = RUNNING;
					}
					
					anim->loop = true;
					std::cout << "\nState: " << getStateName() << std::endl;
				}

				break;
		}
	}

};
