#include "Scene.h"

namespace odin {

	//scene can have no audio (_audioBankName == NULL)
	Scene::Scene(GLuint _program, SDL_Renderer* _renderer, std::string _audioBankName)
		:program(_program),
		audioBankName(_audioBankName),
		renderer(_renderer)
	{
		uMatrix = glGetUniformLocation(_program, "uMatrix");
		uColor = glGetUniformLocation(_program, "uColor");
		uTexture = glGetUniformLocation(_program, "uTexture");
		uFacingDirection = glGetUniformLocation(_program, "uFacingDirection");
		uCurrentFrame = glGetUniformLocation(_program, "uCurrentFrame");
		uCurrentAnim = glGetUniformLocation(_program, "uCurrentAnim");
		uMaxFrame = glGetUniformLocation(_program, "uMaxFrames");
		uMaxAnim = glGetUniformLocation(_program, "uTotalAnim");

		if (!audioBankName.empty()) {
			//each scene will be responsible for loading its own sounds
			audioEngine.loadBank(audioBankName + ".bank", FMOD_STUDIO_LOAD_BANK_NORMAL);
			audioEngine.loadBank(audioBankName + ".strings.bank", FMOD_STUDIO_LOAD_BANK_NORMAL);
		}
	}

	//destructor required to unload audio from engine on destroy
	Scene::~Scene() {
		if (!audioBankName.empty()) //unload the associate bank
			audioEngine.unloadBank(audioBankName + ".bank");
		audioEngine.unloadBank(audioBankName + ".strings.bank");
	}

	void Scene::handleInput() {
		inputManager.pollEvents();
		for ( auto x : listeners )
		{
			trigger( x.value, x.key );
		}
	}

	void Scene::update(float tgtFrameTime) {
		b2world.Step(tgtFrameTime, 50, 50);
        for ( auto x : fsxComponents )
		{
			updateComponent( x.value, x.key );
		}
	}

	void Scene::draw(float zoom, float aspect, GLuint program) {
		for ( auto x : gfxComponents )
		{
			drawComponent( x.value, x.key, zoom, aspect);
		}
	}

	void Scene::updateComponent(const PhysicalComponent& fsx, EntityId eid)
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

	void Scene::drawComponent(GraphicalComponent& gfx, EntityId eid, float zoom, float aspect)
	{
		using namespace glm;
		Entity& entity = entities[eid];

		mat4 mtx;
		mtx = scale(mtx, vec3(zoom, zoom * aspect, 1));
		mtx = translate(mtx, vec3(entity.position.glmvec2, 0));
		mtx = rotate(mtx, entity.rotation, vec3(0, 0, 1));

		gfx.incrementFrame();


		if (gfx.programId == 0)
		{
			glUseProgram(program);
			glUniform(uMatrix, mtx);
			glUniform(uColor, gfx.color);
			glUniform(uTexture, gfx.texture);
			glUniform(uFacingDirection, gfx.direction);
			glUniform(uCurrentFrame, gfx.currentFrame);
			glUniform(uCurrentAnim, gfx.animState);
			glUniform(uMaxFrame, gfx.maxFrames);
			glUniform(uMaxAnim, gfx.totalAnim);
		}

		glBindVertexArray(gfx.vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, gfx.count);
	}

	void Scene::trigger(const InputListener& lstn, EntityId eid)
	{
		lstn(inputManager, eid);
	}


	void Scene::add(EntityId eid, GraphicalComponent gfx)
	{
		gfxComponents.add(eid, std::move(gfx));
	}

	void Scene::add(EntityId eid, PhysicalComponent fsx)
	{
		fsxComponents.add(eid, std::move(fsx));
	}

	void Scene::add(EntityId eid, InputListener lstn)
	{
		listeners.add(eid, std::move(lstn));
	}

	EntityView Scene::addRect(
		EntityId   eid,
		Vec2       dimen,
		Vec2       pos,
		float      rot,
		glm::vec3  color,
		b2BodyType bodyType)
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

	EntityView Scene::addRightTri(EntityId   eid,
		Vec2       dimen,
		Vec2       pos,
		float      rot,
		glm::vec3  color,
		b2BodyType bodyType)
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

	EntityView Scene::addEqTri(EntityId   eid,
		float      length,
		Vec2       pos,
		float      rot,
		glm::vec3  color,
		b2BodyType bodyType)
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