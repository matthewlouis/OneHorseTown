#include "Window.h"
#include "Errors.h" 

namespace OdinEngine {
	Window::Window()
	{
	}


	Window::~Window()
	{
	}


	//Creates an OpenGL window
	int Window::create(std::string windowName, int screenWidth, int screenHeight, unsigned int currentFlags) {

		Uint32 glFlags = SDL_WINDOW_OPENGL;

		if (currentFlags & INVISIBLE) {
			glFlags |= SDL_WINDOW_HIDDEN;
		}
		if (currentFlags & FULLSCREEN) {
			glFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		if (currentFlags & BORDERLESS) {
			glFlags |= SDL_WINDOW_BORDERLESS;
		}


		//Create OpenGL window
		//SDL_sdlWindowPOS_CENTERED creates window in the center position using given width/height
		_sdlWindow = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			screenWidth, screenHeight, glFlags);

		//simple error checking
		if (_sdlWindow == nullptr) {
			fatalError("SDL Window could not be created.");
		}

		SDL_GLContext glContext = SDL_GL_CreateContext(_sdlWindow);
		if (glContext == nullptr) {
			fatalError("SDL_GL context could not be created.");
		}

		GLenum error = glewInit();
		if (error != GLEW_OK) {
			fatalError("Could not init glew.");
		}

		printf("*** OpenGL Version: %s ***", glGetString(GL_VERSION)); //output opengl version for debugging

		glClearColor(0.5f, 0, 0, 1); //set clear color to deep, dark, bloody red

		//Sets VSYNC 0 for off, 1 for on
		SDL_GL_SetSwapInterval(0);

		//allowing transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		return 0;
	}

	void Window::swapBuffer() {
		SDL_GL_SwapWindow(_sdlWindow); //swaps buffer
	}
}