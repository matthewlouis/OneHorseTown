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
//using odin::Scene;

enum PlayerState {
	IDLE = 0,
	RUNNING = 5,
	IN_AIR = 6,
	IDLE_SHOOTING = 7,
	RUNNING_SHOOTING = 5,
	IN_AIR_SHOOTING = 6,
	HIT = 8,
	DEAD = 9
};

class Player {
public:
	static int deadPlayers;
	const float FALL_THRESHOLD = 0.1f;
	//arm
	GraphicalComponent* arm_gfx;
	AnimatorComponent* arm_anim;
    b2Body* arm_psx;
	Vec2 armOffsets[5];

	//player
	GraphicalComponent* gfx;
	AnimatorComponent* anim;
	b2Body* psx;

	PlayerState currentState;
	bool falling = false;
	bool shooting = false;
	bool aiming = false;
	int doubleJump = 0;

	//TODO: keeping track of ammunition
	int bulletCount = 3;
	int killCount = 0;
	int lives = 3;
	bool alive = true;
	
	struct SoundEvent {
		bool playEvent = false;
		std::string event = "";

		SoundEvent() {
			playEvent = false;
			this->event = "";
		}

		SoundEvent(bool playEvent, std::string event) :
			playEvent(playEvent),
			event(event) {}
	}soundEvent;

	bool active = false;

	void init(GraphicalComponent *gfx,
			  AnimatorComponent *anim,
               b2Body *psx,
			  GraphicalComponent *arm_gfx, 
			  AnimatorComponent *arm_anim,
               b2Body *arm_psx) {
		if (active)
			return;

		this->gfx = gfx;
		this->anim = anim;
		this->arm_gfx = arm_gfx;
		this->arm_anim = arm_anim;
		this->arm_anim->play = false;
		this->psx = psx;
		this->arm_psx = arm_psx;

		//TODO: arm offsets for other animation states;
		//set arm offsets so it renders in correct location
		armOffsets[0] = Vec2(0.5, 0.9);
		armOffsets[1] = Vec2(0.6, 0.75);
		armOffsets[2] = Vec2(0.75, 0.475);
		armOffsets[3] = Vec2(0.6, 0.275);
		armOffsets[4] = Vec2(0.25, 0.05);

		active = true;
	}


	//returns the name of the state as a string for debugging
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

	odin::Direction8Way aimArm(Vec2 aimDir) {
		//calculate angle of aim using aimDir
		float aimAngle = atan2(aimDir.y, aimDir.x);
		odin::Direction8Way aimDirection = odin::calculateDirection8Way(aimAngle);

		//choose the correct arm animation based on direction
		switch (aimDirection) {
		case(odin::EAST) :
		case(odin::WEST) :
			arm_anim->animState = 2;
			break;
		case(odin::NORTH_EAST) :
		case(odin::NORTH_WEST) :
			arm_anim->animState = 1;
			break;
		case(odin::SOUTH_EAST) :
		case(odin::SOUTH_WEST) :
			arm_anim->animState = 3;
			break;
		case(odin::NORTH) :
			arm_anim->animState = 0;
			break;
		case(odin::SOUTH) :
			arm_anim->animState = 4;
			break;
		}

		//adjust facing direction for joystick
		if (aimDir.x < 0) {
			gfx->direction = odin::LEFT;
			arm_gfx->direction = odin::LEFT;
		}
		if (aimDir.x > 0) {
			gfx->direction = odin::RIGHT;
			arm_gfx->direction = odin::RIGHT;

		}

		// Correct default aim direction if no aim present
		if (aimAngle == 0)
		{
			if (gfx->direction == odin::LEFT)
				aimDirection = odin::Direction8Way::WEST;
			else if (gfx->direction == odin::RIGHT)
				aimDirection = odin::Direction8Way::EAST;

			aiming = false;
		}
		else {
			aiming = true;
		}

		return aimDirection;
	}

	void update() {
		if (!active)
			return;

		//Determine Player state
		Vec2 vel = psx->GetLinearVelocity();

		switch (currentState) {
			case IDLE:
				if (vel.y > FALL_THRESHOLD || vel.y < -FALL_THRESHOLD) {
					anim->switchAnimState(IN_AIR);
					anim->loop = false;
					currentState = IN_AIR;
				}
				else if (abs(vel.x) > FALL_THRESHOLD) {
					anim->switchAnimState(RUNNING);
					anim->loop = true;
					currentState = RUNNING;
				}
				break;
			case RUNNING:
				if (vel.y > FALL_THRESHOLD || vel.y < -FALL_THRESHOLD) {
					anim->switchAnimState(IN_AIR);
					anim->loop = false;
					currentState = IN_AIR;
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
				if (vel.y < -FALL_THRESHOLD)
					falling = true;

				if (falling && vel.y <= FALL_THRESHOLD && vel.y >= -FALL_THRESHOLD) {
					falling = false;
					
					if (abs(vel.x) < FALL_THRESHOLD) {
						anim->switchAnimState(IDLE);
						currentState = IDLE;
						doubleJump = 0;
					}
					else {
						anim->switchAnimState(RUNNING);
						anim->currentFrame = 4;
						currentState = RUNNING;
						doubleJump = 0;
					}
					
					anim->loop = true;
				}

				break;
		}


		if (aiming) {
			arm_gfx->visible = true;
			//Determine arm state
			Vec2 armPosition = psx->GetPosition();

			//current state determines the arm offset
			int currentState = arm_anim->animState;

			armPosition.y += armOffsets[currentState].y;
			armPosition.x += gfx->direction * armOffsets[currentState].x;

			arm_psx->SetTransform(armPosition, 0);
		}
		else {
			arm_gfx->visible = false;
		}
	}

	//called when player is killed
	void died() {
		if (--lives <= 0) {
			alive = false;
		}
	}

	void countKill() {
		killCount++;
		Player::deadPlayers++;
	}

};
