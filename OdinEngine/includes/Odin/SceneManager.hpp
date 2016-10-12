#pragma once

#include "Entity.hpp"

namespace odin {
	class Scene;

	// Lazy entity proxy object.
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

		virtual void setup_scene(){}

		EntityView addRect(EntityId   eid,
			Vec2       dimen,
			Vec2       pos = { 0, 0 },
			float      rot = 0,
			glm::vec3  color = { 1, 1, 1 },
			b2BodyType bodyType = b2BodyType::b2_staticBody)
		{
			if (!entities.add(eid, Entity(pos, rot)))
				std::cout << "Entity " << eid << " already exists.\n";

			if (!gfxComponents.add(eid,
				GraphicalComponent::makeRect(dimen.x, dimen.y, color)))
				std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

			b2BodyDef bodyDef;
			bodyDef.position = pos;
			bodyDef.angle = rot;
			bodyDef.type = bodyType;

			if (!fsxComponents.add(eid,
				PhysicalComponent::makeRect(dimen.x, dimen.y, b2world, bodyDef)))
				std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";
			return EntityView(eid, this);
		}

		EntityView addRightTri(EntityId   eid,
			Vec2       dimen,
			Vec2       pos = { 0, 0 },
			float      rot = 0,
			glm::vec3  color = { 1, 1, 1 },
			b2BodyType bodyType = b2BodyType::b2_staticBody)
		{
			if (!entities.add(eid, Entity(pos, rot)))
				std::cout << "Entity " << eid << " already exists.\n";

			if (!gfxComponents.add(eid,
				GraphicalComponent::makeRightTri(dimen.x, dimen.y, color)))
				std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

			b2BodyDef bodyDef;
			bodyDef.position = pos;
			bodyDef.angle = rot;
			bodyDef.type = bodyType;

			if (!fsxComponents.add(eid,
				PhysicalComponent::makeRightTri(dimen.x, dimen.y, b2world, bodyDef)))
				std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";

			return EntityView(eid, this);

		}

		EntityView addEqTri(EntityId   eid,
			float      length,
			Vec2       pos = { 0, 0 },
			float      rot = 0,
			glm::vec3  color = { 1, 1, 1 },
			b2BodyType bodyType = b2BodyType::b2_staticBody)
		{
			if (!entities.add(eid, Entity(pos, rot)))
				std::cout << "Entity " << eid << " already exists.\n";

			if (!gfxComponents.add(eid, GraphicalComponent::makeEqTri(length, color)))
				std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

			b2BodyDef bodyDef;
			bodyDef.position = pos;
			bodyDef.angle = rot;
			bodyDef.type = bodyType;

			if (!fsxComponents.add(eid, PhysicalComponent::makeEqTri(length, b2world, bodyDef)))
				std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";

			return EntityView(eid, this);
		}

		void handleInput() {
			inputManager.pollEvents();
			for (auto itr = listeners.begin();
			itr != listeners.end(); ++itr)
			{
				trigger(*itr, itr.key());
			}
		}

		void update(float tgtFrameTime) {
			b2world.Step(tgtFrameTime, 50, 50);
			for (auto itr = fsxComponents.begin();
			itr != fsxComponents.end(); ++itr)
			{
				updateComponent(*itr, itr.key());
			}
		}

		void draw(float zoom, float aspect, GLuint program) {
			for (auto itr = gfxComponents.begin();
			itr != gfxComponents.end(); ++itr)
			{
				drawComponent(*itr, itr.key(), zoom, aspect, program);
			}
		}

		void updateComponent(const PhysicalComponent& fsx, EntityId eid)
		{
			Entity& entity = entities[eid];

			entity.position = fsx.position();

			if (fsx.pBody->IsBullet())
			{
				Vec2 vel = fsx.pBody->GetLinearVelocity();

				float angle = std::atan2(vel.y, vel.x);
				fsx.pBody->SetTransform(entity.position, angle);
			}

			entity.rotation = fsx.rotation();
		}


		void drawComponent(const GraphicalComponent& gfx, EntityId eid, float zoom, float aspect, GLuint program)
		{
			using namespace glm;
			Entity& entity = entities[eid];

			mat4 mtx;
			mtx = scale(mtx, vec3(zoom, zoom * aspect, 1));
			mtx = translate(mtx, vec3(entity.position.glmvec2, 0));
			mtx = rotate(mtx, entity.rotation, vec3(0, 0, 1));

			GLint uMatrix = glGetUniformLocation(program, "uMatrix");
			GLint uColor = glGetUniformLocation(program, "uColor");
			GLint uTexture = glGetUniformLocation(program, "uTexture");

			if (gfx.programId == 0)
			{
				glUseProgram(program);
				glUniform(uMatrix, mtx);
				glUniform(uColor, gfx.color);
				glUniform(uTexture, gfx.texture);
			}

			glBindVertexArray(gfx.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, gfx.count);
		}

		void trigger(const InputListener& lstn, EntityId eid)
		{
			lstn(inputManager, eid);
		}


		void add(EntityId eid, GraphicalComponent gfx)
		{
			gfxComponents.add(eid, std::move(gfx));
		}

		void add(EntityId eid, PhysicalComponent fsx)
		{
			fsxComponents.add(eid, std::move(fsx));
		}

		void add(EntityId eid, InputListener lstn)
		{
			listeners.add(eid, std::move(lstn));
		}


	};

	class SceneManager {
	public:
		int currentScene = -1;
		std::vector<Scene*> scenes;

		~SceneManager() {
			for (auto itr = scenes.begin();
			itr != scenes.end(); ++itr)
			{
				delete *itr;
			}
		}

		void changeScene(int newScene) {
			if (newScene < scenes.size()) {
				currentScene = newScene;
			}
		}

		void addScene(int idx, Scene* scene) {
			if (idx >= scenes.size())
				scenes.push_back(scene);
			else {
				scenes[idx] = scene;
			}

			scene->setup_scene();
		}

		void draw(float zoom, float aspect, GLuint program) {
			if(currentScene >= 0 && currentScene < scenes.size())
				scenes[currentScene]->draw(zoom, aspect, program);
		}

		void handleInput() {
			if (currentScene >= 0 && currentScene < scenes.size())
				scenes[currentScene]->handleInput();
		}

		void update(float tgtFrameTime) {
			if (currentScene >= 0 && currentScene < scenes.size())
				scenes[currentScene]->update(tgtFrameTime);
		}
	};

	void EntityView::attach(GraphicalComponent gfx)
	{
		pScene->gfxComponents[eid] = std::move(gfx);
	}

	void EntityView::attach(PhysicalComponent fsx)
	{
		pScene->fsxComponents[eid] = std::move(fsx);
	}

	void EntityView::attach(InputListener lstn)
	{
		pScene->listeners[eid] = std::move(lstn);
	}

	void EntityView::detach(ComponentType type)
	{
		switch (type)
		{
		case ComponentType::Graphical:
			pScene->gfxComponents.remove(eid);
			break;
		case ComponentType::Physical:
			pScene->fsxComponents.remove(eid);
			break;
		case ComponentType::Input:
			pScene->listeners.remove(eid);
			break;
		}
	}

	GraphicalComponent* EntityView::gfxComponent()
	{
		auto itr = pScene->gfxComponents.search(eid);
		return itr ? (GraphicalComponent*)itr : nullptr;
	}

	PhysicalComponent* EntityView::fsxComponent()
	{
		auto itr = pScene->fsxComponents.search(eid);
		return itr ? (PhysicalComponent*)itr : nullptr;
	}

	InputListener* EntityView::inputListener()
	{
		auto itr = pScene->listeners.search(eid);
		return itr ? (InputListener*)itr : nullptr;
	}
}