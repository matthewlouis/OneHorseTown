#pragma once

#include "Scene.h"
#include "AudioEngine.h"

namespace odin {
	class SceneManager {
	public:
		size_t currentScene = -1;
		std::vector<Scene*> scenes;

		~SceneManager() {
			for (auto itr = scenes.begin();
			itr != scenes.end(); ++itr)
			{
				delete *itr;
			}
		}

		void changeScene(size_t newScene) {
			if (newScene < scenes.size()) {
				currentScene = newScene;
			}
		}

		void addScene(size_t idx, Scene* scene) {
			if (idx >= scenes.size())
				scenes.push_back(scene);
			else {
				scenes[idx] = scene;
			}

			scene->setup_scene();
		}

		void draw(float zoom, float aspect, GLuint program) {
			if(currentScene != -1 && currentScene < scenes.size())
				scenes[currentScene]->draw(zoom, aspect, program);
		}

		void handleInput() {
			if (currentScene != -1 && currentScene < scenes.size())
				scenes[currentScene]->handleInput();
		}

		void update(float tgtFrameTime) {
			if (currentScene != -1 && currentScene < scenes.size())
				scenes[currentScene]->update(tgtFrameTime);
		}
	};

}