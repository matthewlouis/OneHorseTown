#pragma once

namespace OdinEngine {

	class FPSLimiter {
	public:
		FPSLimiter();
		void init(float maxFPS);

		void setMaxFPS(float maxFPS);
		//start fps limitting
		void beginFrame();

		//returns the fps
		float endFrame(); 

	private:
		void calculateFPS();

		float _maxFPS;
		unsigned int _startTicks;
		float _fps;
		float _frameTime;
	};
}