#pragma once

#include "TestScene.hpp"
#include "LobbyScene.hpp"

class TitleScene
	: public odin::Scene
{
public:

	template< typename ValueType >
	using EntityMap = odin::BinarySearchMap< EntityId, ValueType >;

	EntityMap< Entity >             entities;

	EntityMap< GraphicalComponent > gfxComponents;
	EntityMap< AnimatorComponent > animComponents;

	b2ThreadPool                    b2thd;
	b2World                         b2world = { { 0.f, -9.81f }, &b2thd };
	EntityMap< PhysicalComponent >  fsxComponents;

	InputManager*                   pInputManager;
	std::vector< InputListener >    listeners;

	std::string                     audioBankName;
	AudioEngine*                    pAudioEngine;

    odin::SceneManager* pSceneManager;

	OHT_DEFINE_COMPONENTS(entities, gfxComponents, animComponents, fsxComponents);

	//SDL_Renderer* renderer;

	EntityId promptID;

	// Variables for blinking animation
	unsigned OFF_TIME = 40, ON_TIME = 55;
	unsigned onFrame = 0, offFrame = 0;
	bool promptOn = true;
	bool buttonPressed = false;

	unsigned int TIME_BEFORE_INTRO = 15000;
	bool introStarted = false;
	int currentSlide = 0;
	int slideTimes[14] = { 0, 3000, 3000, 1500, 1500, 2500, 700, 700, 700, 2000, 500, 500, 500, 500 };
	GraphicalComponent* background;
	AnimatorComponent* backgroundAnim;
	int fadeTime = 250;
	bool fadedOut = false;
	bool fading = false;
	bool goingBackToTitle = false;
	uint32 timeAtStartScreen = 0;
	float fadetest = 0;

	GLuint program;
	GLint uMatrix, uColor, uTexture, uFacingDirection,
		uCurrentFrame, uCurrentAnim, uMaxFrame, uMaxAnim,
		uFadeOut;

	TitleScene( int width, int height, std::string audioBank = "")
		: Scene(width, height)
		, audioBankName(std::move(audioBank))
		, program(load_shaders("Shaders/vertexAnim.glsl", "Shaders/fragmentShader.glsl"))
		, uMatrix(glGetUniformLocation(program, "uMatrix"))
		, uColor(glGetUniformLocation(program, "uColor"))
		, uTexture(glGetUniformLocation(program, "uTexture"))
		, uFacingDirection(glGetUniformLocation(program, "uFacingDirection"))
		, uCurrentFrame(glGetUniformLocation(program, "uCurrentFrame"))
		, uCurrentAnim(glGetUniformLocation(program, "uCurrentAnim"))
		, uMaxFrame(glGetUniformLocation(program, "uMaxFrames"))
		, uMaxAnim(glGetUniformLocation(program, "uTotalAnim"))
		, uFadeOut(glGetUniformLocation(program, "uFadeOut"))
	{
	}

	//fade out scene, and return true when complete, must be called every update
	bool fadeout(int startTime, int currentTime, int fadeLength) {
		int timePassed = currentTime - startTime;

		//linear fade
		float fadeAmount = (float)timePassed / fadeLength;
		fadeAmount = fadeAmount > 1.0f ? 1.0 : fadeAmount; //clamp if greater than 1;

		fadetest = fadeAmount;

		glUniform(uFadeOut, fadeAmount);

		if (fadeAmount >= 1.0f) {
			return true;
		}
		else {
			return false;
		}
	}

	//fade in scene, and return true when complete, must be called every update
	bool fadein(int startTime, int currentTime, int fadeLength) {
		int timePassed = currentTime - startTime;

		//linear fade
		float fadeAmount = 1.0f - (float)timePassed / fadeLength;
		fadeAmount = fadeAmount < 0.0f ? 0.0 : fadeAmount; //clamp if greater than 1;

		fadetest = fadeAmount;

		glUniform(uFadeOut, fadeAmount);

		if (fadeAmount <= 0.0f) {
			return true;
		}
		else {
			return false;
		}
	}

	//reset to initial state
	void reset(unsigned ticks) {
		timeAtStartScreen = ticks;
		buttonPressed = false;
		introStarted = false;
		currentSlide = 0;
		fadedOut = false;
		fading = false;
		goingBackToTitle = false;
		background->texture = TITLE;
		OFF_TIME = 40;
		ON_TIME = 55;
		promptOn = true;
		gfxComponents[promptID].color.w = 1;
		offFrame = onFrame = 0;
		glUniform(uFadeOut, 0.0f);
	}

	void pause(unsigned ticks) override {
		glUniform(uFadeOut, 0.0f);
		pAudioEngine->stopEvent("event:/Music/IntroTheme");
		Scene::pause(ticks);
	}

	void resume(unsigned ticks) override {
		Scene::resume(ticks);
		pAudioEngine->playEvent("event:/Music/IntroTheme");
		//reset all conditions for intro
		reset(ticks);
	}

	void init(unsigned ticks)
	{
		Scene::init(ticks);

		odin::load_texture(TITLE, "Textures/title.png");
		odin::load_texture(PRESS_BUTTON, "Textures/pressbutton.png");

		background = gfxComponents.add(
			EntityId(0), GraphicalComponent::makeRect(width, height));
		background->texture = TITLE;
		background->interactive = false;
		
		//adding the 12 slide animation sprite sheet (1 frame per slide)
		backgroundAnim = animComponents.add(EntityId(0), AnimatorComponent::AnimatorComponent({ 1,1,1,1,1,1,1,1,1,1,1,1,1 }));
		backgroundAnim->switchAnimState(currentSlide);
		
		
		promptID = EntityId(1);
		auto prompt = gfxComponents.add(promptID, GraphicalComponent::makeRect(120, 9));
		prompt->texture = PRESS_BUTTON;
		prompt->interactive = false;

		Vec2 pos = { 160, -55 };
		if (!entities.add(promptID, Entity(pos, 0)))
		   std::cout << "Entity " << promptID << " already exists.\n";

		listeners.push_back([this](const InputManager& inmn) {
			if (inmn.wasKeyPressed(SDLK_RETURN) || inmn.gamepads.wasButtonPressed(0, SDL_CONTROLLER_BUTTON_START)) {
				buttonPressed = true;
			}
		});

		listeners.push_back([this](const InputManager& inmn) {
			if (inmn.wasKeyPressed(SDLK_m))
				pAudioEngine->toggleMute();
		});

		if (audioBankName != "")
		{
			pAudioEngine->loadBank(audioBankName + ".bank",
				FMOD_STUDIO_LOAD_BANK_NORMAL);
			pAudioEngine->loadBank(audioBankName + ".strings.bank",
				FMOD_STUDIO_LOAD_BANK_NORMAL);
		}
		//load menu sound and music
		pAudioEngine->loadEvent("event:/UI/Menu Select");
		pAudioEngine->loadEvent("event:/Music/IntroTheme");
		
		//play music
		pAudioEngine->playEvent("event:/Music/IntroTheme");
	}

	void exit(unsigned ticks)
	{
		Scene::exit(ticks);

		/* Using 1 bank for all audio so don't unload
		if (audioBankName != "")
		{
			pAudioEngine->unloadBank(audioBankName + ".bank");
			pAudioEngine->unloadBank(audioBankName + ".strings.bank");
		}*/
	}

	void update(unsigned ticks)
	{
		if (ticks - timeAtStartScreen > TIME_BEFORE_INTRO && !buttonPressed) {
			introStarted = true;
		}


		//Do slideshow intro
		if (introStarted) {
			static int slideStartTime = ticks;
			
			offFrame = 0;
			promptOn = false;

			if (SDL_GetTicks() - slideStartTime > slideTimes[currentSlide]) { //if enough time has passed

				static unsigned int fadeStartTime = ticks;
				if (!fading) {
					fadeStartTime = ticks;
					fading = true;
				}

				//check if slide has faded out, if so change the slide
				if (!fadedOut && fadeout(fadeStartTime, ticks, fadeTime)) {
					fadedOut = true;
					gfxComponents[promptID].color.w = 0;

					if (currentSlide >= 12) {
						currentSlide = 0;
						backgroundAnim->switchAnimState(currentSlide);
					}
					else {
						backgroundAnim->switchAnimState(++currentSlide);
					}			
					fadeStartTime = ticks; //reset fade start to do fade-in
				}
				else if (fadedOut && fadein(fadeStartTime, ticks, fadeTime)) {
					fadedOut = false;
					slideStartTime = ticks;

					fading = false;
					if (currentSlide == 0) { //we've just faded back into the title screen
						introStarted = false;
						timeAtStartScreen = slideStartTime;
					}
				}
			}
		}


		if (buttonPressed && !introStarted) {
			static int startedTicks = ticks;
			if (!fading) {
				fading = true;
				startedTicks = ticks;
				pAudioEngine->playEvent("event:/UI/Menu Select");
			}

			ON_TIME = OFF_TIME = 4;
			
			if (fadeout(startedTicks, ticks, 2000)) {
				fading = false;

                auto lobby = new LobbyScene( width, height ) ; //1 player
                lobby->pInputManager = pInputManager;
                lobby->pAudioEngine = pAudioEngine;
                lobby->pSceneManager = pSceneManager;
                pSceneManager->pushScene( lobby );

                //auto level = new TestScene( width, height, SCALE, 4); //1 player
                //level->pInputManager = pInputManager;
                //level->pAudioEngine = pAudioEngine;
                //level->pSceneManager = pSceneManager;
                //pSceneManager->pushScene( level );

                buttonPressed = false;
				//this->expired = true;
			}
		}
		else if (buttonPressed && introStarted) {
			static int startedTicks = ticks;
			if (!goingBackToTitle) {
				startedTicks = ticks;
				goingBackToTitle = true;
			}

			if (fadeout(startedTicks, ticks, 500)) {
				//reset everything to original state to go back to title
				reset(ticks);
			}
		}

		for (auto x : animComponents)
		{
			x.value.incrementFrame();
			auto& texAdjust = entities[x.key].texAdjust;

			texAdjust[0] = x.value.animState;
			texAdjust[1] = x.value.currentFrame;
			texAdjust[2] = x.value.maxFrames;
			texAdjust[3] = x.value.totalAnim;
			switch (x.value.type)
			{
				// Handle fading animation
			case odin::AnimationType::FADEOUT:
				if (x.value.currentFrame == x.value.maxFrames - 1) {
					gfxComponents.remove(x.key);
					animComponents.remove(x.key);
					entities.remove(x.key);
				}
				else {
					gfxComponents[x.key].color = { 1,1,1, 1.f - (float)x.value.currentFrame / (float)x.value.maxFrames };
				}
			default:
				break;
			}
		}

		Scene::update(ticks);

		for ( auto& lstn : listeners )
			lstn( *pInputManager );
	}

	void draw()
	{
		using namespace glm;
		Scene::draw();

		float zoom = 1.0f / SCALE;
		float aspect = width / (float)height;
		const mat4 base = scale({}, vec3(zoom, zoom * aspect, 1));

		if (promptOn) {
			++onFrame;
			if (onFrame > ON_TIME) {
				gfxComponents[promptID].color.w = 0;
				promptOn = false;
				onFrame = 0;
			}
		}else {
			++offFrame;
			if (offFrame > OFF_TIME) {
				gfxComponents[promptID].color.w = 1;
				promptOn = true;
				offFrame = 0;
			}
		}

		glUseProgram(program);
		for (auto x : gfxComponents)
		{

			Entity& ntt = entities[x.key];
			auto& gfx = x.value;

			if (!gfx.visible)
				continue;

			mat4 mtx = translate(base, vec3(ntt.position.glmvec2, 0));
			mtx = rotate(mtx, ntt.rotation, vec3(0, 0, 1));

			glUniform(uMatrix, mtx);
			glUniform(uColor, gfx.color);
			glUniform(uTexture, gfx.texture);
			glUniform(uFacingDirection, gfx.direction);

            glUniform( uCurrentAnim, ntt.texAdjust[ 0 ] );
            glUniform( uCurrentFrame, ntt.texAdjust[ 1 ] );
            glUniform( uMaxFrame, ntt.texAdjust[ 2 ] );
            glUniform( uMaxAnim, ntt.texAdjust[ 3 ] );

			glBindVertexArray(gfx.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, gfx.count);
		}
		//Scene::endDraw(); //for post processing
	}

	void add(EntityId eid, GraphicalComponent gfx)
	{
		gfxComponents.add(eid, std::move(gfx));
	}

	void add(EntityId eid, PhysicalComponent fsx)
	{
		fsxComponents.add(eid, std::move(fsx));
	}

	void add(InputListener lstn)
	{
		listeners.push_back(std::move(lstn));
	}
};