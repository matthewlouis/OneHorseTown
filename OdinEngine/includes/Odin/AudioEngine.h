/** 
 * AudioEngine is the main audio manager, using FMOD for playback and event support.
 * Based off tutorial found at:
 * http://codyclaborn.me/2016/04/12/making-a-basic-fmod-audio-engine-in-c/#audio-engine-source-code
 **/
#pragma once

#include "FMOD\fmod_studio.hpp"
#include "FMOD\fmod.hpp"
#include <string>
#include <map>
#include <vector>
#include <math.h>
#include <iostream>
#include <glm\glm.hpp>

namespace OdinEngine {
	struct Implementation {
		Implementation();
		~Implementation();

		void Update();

		FMOD::Studio::System* mpStudioSystem;
		FMOD::System* mpSystem;

		int mnNextChannelId;

		typedef std::map<std::string, FMOD::Sound*> SoundMap;
		typedef std::map<int, FMOD::Channel*> ChannelMap;
		typedef std::map<std::string, FMOD::Studio::EventInstance*> EventMap;
		typedef std::map<std::string, FMOD::Studio::Bank*> BankMap;

		BankMap mBanks;
		EventMap mEvents;
		SoundMap mSounds;
		ChannelMap mChannels;
	};

	class AudioEngine {
	public:
		static void init();
		static void update();
		static void shutdown();

		void loadBank(const std::string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
		void loadEvent(const std::string& strEventName);
		void loadSound(const std::string& strSoundName, bool is3d = true, bool isLooping = false, bool isStreaming = false);
		void unLoadSound(const std::string& strSoundName);

		int  playSound(const std::string& strSoundName, const glm::vec3& vPos = glm::vec3(0, 0, 0), float fVolumedB = 0.0f);
		void playEvent(const std::string& strEventName);

		void getEventParameter(const std::string& strEventName, const std::string& strEventParameter, float* parameter);
		void setEventParameter(const std::string& strEventName, const std::string& strParameterName, float fValue);

		void stopChannel(int nChannelId);
		void stopEvent(const std::string& strEventName, bool bImmediate = false);
		void stopAllChannels();

		void setChannel3dPosition(int nChannelId, const glm::vec3& vPosition);
		void setChannelVolume(int nChannelId, float fVolumedB);
		void set3dListenerPosition(const glm::vec3& vPos = glm::vec3(0, 0, 0));

		bool isChannelPlaying(int nChannelId) const;
		bool isEventPlaying(const std::string& strEventName) const;

		//Utility methods
		float dbToVolume(float db); //db to volume conversion
		float volumeTodB(float volume); //volume to db conversion
		FMOD_VECTOR vectorToFmod(const glm::vec3& vPosition); //convert glm vector to FMOD vector
	};
}