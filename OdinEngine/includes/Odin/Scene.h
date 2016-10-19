#pragma once
#include "Entity.hpp"
#include "AudioEngine.h"

namespace odin {
	// Lazy entity proxy object.
	struct EntityView;

	class Scene {
	public:
		template< typename ValueType >
		using EntityMap = odin::BinarySearchMap< EntityId, ValueType >;


		EntityMap< Entity >             entities;

		EntityMap< GraphicalComponent > gfxComponents;

		b2ThreadPool                    b2thd;
		b2World                         b2world = { { 0.f, -9.81f }, &b2thd };
		EntityMap< PhysicalComponent >  fsxComponents;

		InputManager                    inputManager;
		EntityMap< InputListener >      listeners;

		std::string audioBankName;
		AudioEngine audioEngine;

		SDL_Renderer* renderer;

		GLuint program;
		GLint uMatrix, uColor, uTexture, uFacingDirection,
			uCurrentFrame, uCurrentAnim, uMaxFrame, uMaxAnim;

		virtual void setup_scene() {}

		//scene can have no audio (_audioBankName == NULL)
		Scene(GLuint _program, SDL_Renderer* _renderer, std::string _audioBankName = NULL);
		~Scene();

		EntityView addRect(EntityId   eid,
			Vec2       dimen,
			Vec2       pos = { 0, 0 },
			float      rot = 0,
			glm::vec3  color = { 1, 1, 1 },
			b2BodyType bodyType = b2BodyType::b2_staticBody);

		EntityView addRightTri(EntityId   eid,
			Vec2       dimen,
			Vec2       pos = { 0, 0 },
			float      rot = 0,
			glm::vec3  color = { 1, 1, 1 },
			b2BodyType bodyType = b2BodyType::b2_staticBody);

		EntityView addEqTri(EntityId   eid,
			float      length,
			Vec2       pos = { 0, 0 },
			float      rot = 0,
			glm::vec3  color = { 1, 1, 1 },
			b2BodyType bodyType = b2BodyType::b2_staticBody);

		void handleInput();

		void update(float tgtFrameTime);

		void draw(float zoom, float aspect, GLuint program);

		void updateComponent(const PhysicalComponent& fsx, EntityId eid);


		void drawComponent(GraphicalComponent& gfx, EntityId eid, float zoom, float aspect);

		void trigger(const InputListener& lstn, EntityId eid);

		void add(EntityId eid, GraphicalComponent gfx);

		void add(EntityId eid, PhysicalComponent fsx);

		void add(EntityId eid, InputListener lstn);

		virtual void player_input(const InputManager&, EntityId eid){}

	};

	struct EntityView
	{
		EntityId eid;
		Scene*    pScene;

		EntityView(EntityId eid, Scene* game)
			: eid(eid), pScene(game)
		{
		}

		void attach(GraphicalComponent gfx);
		void attach(PhysicalComponent fsx);
		void attach(InputListener lstn);

		void detach(ComponentType type);

		GraphicalComponent* gfxComponent();
		PhysicalComponent* fsxComponent();
		InputListener* inputListener();
	};


}