#include "AudioEngine.h"
#include "Errors.h"

namespace odin {

	Implementation::Implementation() {
		mpStudioSystem = NULL;
		//set up the fmod system
		fmodErrorCheck(FMOD::Studio::System::create(&mpStudioSystem));
		//link up to studio for live mixing
		fmodErrorCheck(mpStudioSystem->initialize(32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, NULL));

		mpSystem = NULL;
		//required for low level access
		fmodErrorCheck(mpStudioSystem->getLowLevelSystem(&mpSystem));
	}

	//clean up
	Implementation::~Implementation() {
		fmodErrorCheck(mpStudioSystem->unloadAll());
		fmodErrorCheck(mpStudioSystem->release());
	}

	void Implementation::Update() {
		std::vector<ChannelMap::iterator> pStoppedChannels;

		//iterate through the channels,
		for (auto it = mChannels.begin(), itEnd = mChannels.end(); it != itEnd; ++it)
		{
			bool bIsPlaying = false;
			it->second->isPlaying(&bIsPlaying);
			if (!bIsPlaying)
			{
				pStoppedChannels.push_back(it); //if channel not playing, add to vector
			}
		}

		//clear stopped channels to make room for new ones
		for (auto& it : pStoppedChannels)
		{
			mChannels.erase(it);
		}
		fmodErrorCheck(mpStudioSystem->update());
	}


	//Audio Engine implementation (static global pointer)
	Implementation* _sgpImplementation = nullptr;

	//simple boolean value to mute audio
	bool AudioEngine::_mute = false;

	void AudioEngine::init() {
		_sgpImplementation = new Implementation;
	}

	void AudioEngine::update() {
		_sgpImplementation->Update();
	}

	//load a sound into memory
	void AudioEngine::loadSound(const std::string& strSoundName, bool is3d, bool isLooping, bool isStreaming)
	{
		//check if already loaded into memory
		auto tFoundIt = _sgpImplementation->mSounds.find(strSoundName);
		if (tFoundIt != _sgpImplementation->mSounds.end())
			return; //don't load

		//describes the sound
		FMOD_MODE fmodMode = FMOD_DEFAULT;
		fmodMode |= is3d ? FMOD_3D : FMOD_2D; //2d or 3d positional sound
		fmodMode |= isLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF; //forward looping or not
		fmodMode |= isStreaming ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE; //decompress at runtime (stream from source) or decompress at loadtime

		FMOD::Sound* pSound = nullptr;
		//create sound with no DSP and add it
		fmodErrorCheck(_sgpImplementation->mpSystem->createSound(strSoundName.c_str(), fmodMode, nullptr, &pSound));
		if (pSound) {
			_sgpImplementation->mSounds[strSoundName] = pSound;
		}
	}


	//unload a bank and free memory
	void AudioEngine::unloadBank(const std::string& strBankName)
	{
		//if sound is not in memory
		auto tFoundIt = _sgpImplementation->mBanks.find(strBankName);
		if (tFoundIt == _sgpImplementation->mBanks.end())
			return; //do nothing

		//release memory and remove from map
		fmodErrorCheck(tFoundIt->second->unload());
		_sgpImplementation->mBanks.erase(tFoundIt);
	}

	//Unload a sound and free the memory
	void AudioEngine::unloadSound(const std::string& strSoundName)
	{
		//if sound is not in memory
		auto tFoundIt = _sgpImplementation->mSounds.find(strSoundName);
		if (tFoundIt == _sgpImplementation->mSounds.end())
			return; //do nothing

		//release memory and remove from map
		fmodErrorCheck(tFoundIt->second->release());
		_sgpImplementation->mSounds.erase(tFoundIt);
	}

	//Plays a sound, and returns the channel it is loaded on
	int AudioEngine::playSound(const std::string& strSoundName, const glm::vec3& vPosition, float fVolumedB)
	{
		int channelId = _sgpImplementation->mnNextChannelId++;

		//look if sound is loaded
		auto tFoundIt = _sgpImplementation->mSounds.find(strSoundName);
		if (tFoundIt == _sgpImplementation->mSounds.end()) //if not loaded
		{
			loadSound(strSoundName); //load the sound

			//check again if loaded okay
			tFoundIt = _sgpImplementation->mSounds.find(strSoundName);
			if (tFoundIt == _sgpImplementation->mSounds.end())
			{
				return channelId; //something went wrong. return channel id
			}
		}

		//sound found. create new channel, but pause the sound so we can set it up
		//(failing to pause sound before setup can result in nasty pops & clicks)
		FMOD::Channel* pChannel = nullptr;
		fmodErrorCheck(_sgpImplementation->mpSystem->playSound(tFoundIt->second, nullptr, true, &pChannel));

		//if channel created successfully
		if (pChannel)
		{
			FMOD_MODE currMode;
			tFoundIt->second->getMode(&currMode);
			if (currMode & FMOD_3D) { //if 3d positional audio, set position
				FMOD_VECTOR position = vectorToFmod(vPosition);
				fmodErrorCheck(pChannel->set3DAttributes(&position, nullptr));
			}
			fmodErrorCheck(pChannel->setVolume(dbToVolume(fVolumedB)));
			fmodErrorCheck(pChannel->setPaused(false)); //unpause audio
			_sgpImplementation->mChannels[channelId] = pChannel;
		}
		return channelId;
	}


	//Set 3d audio position to vector coords
	void AudioEngine::setChannel3dPosition(int nChannelId, const glm::vec3& vPosition)
	{
		//look for channel
		auto tFoundIt = _sgpImplementation->mChannels.find(nChannelId);
		if (tFoundIt == _sgpImplementation->mChannels.end())
			return; //not found

		FMOD_VECTOR position = vectorToFmod(vPosition);
		fmodErrorCheck(tFoundIt->second->set3DAttributes(&position, NULL)); //set position
	}

	//set audio to specific db level
	void AudioEngine::setChannelVolume(int nChannelId, float fVolumedB)
	{
		//look for channel
		auto tFoundIt = _sgpImplementation->mChannels.find(nChannelId);
		if (tFoundIt == _sgpImplementation->mChannels.end())
			return; //channel not found

		fmodErrorCheck(tFoundIt->second->setVolume(dbToVolume(fVolumedB))); //set volume
	}

	//set 3d listener position. As of now, only supports one listener
	void AudioEngine::set3dListenerPosition(const glm::vec3& vPos) {

		FMOD_VECTOR position = vectorToFmod(vPos);
		//sets new position of 3dListener without changing other 3d attributes
		fmodErrorCheck(_sgpImplementation->mpSystem->set3DListenerAttributes(0, &position, 0, 0, 0));
	}


	//The following methods are for FMOD event handling

	//loads an FMOD bank
	void AudioEngine::loadBank(const std::string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags) {
		//check if bank already loaded
		auto tFoundIt = _sgpImplementation->mBanks.find(strBankName);
		if (tFoundIt != _sgpImplementation->mBanks.end())
			return; //if so, return

		//load bank file and add to bank map if successful
		FMOD::Studio::Bank* pBank;
		fmodErrorCheck(_sgpImplementation->mpStudioSystem->loadBankFile(strBankName.c_str(), flags, &pBank));
		if (pBank) {
			_sgpImplementation->mBanks[strBankName] = pBank;
		}
	}

	//loads an FMOD event
	void AudioEngine::loadEvent(const std::string& strEventName) {
		//check if event has been added
		auto tFoundit = _sgpImplementation->mEvents.find(strEventName);
		if (tFoundit != _sgpImplementation->mEvents.end())
			return; //has been found, so return

		//getEvent gets the eventDescription for an event
		FMOD::Studio::EventDescription* pEventDescription = NULL;
		fmodErrorCheck(_sgpImplementation->mpStudioSystem->getEvent(strEventName.c_str(), &pEventDescription));
		if (pEventDescription) { //if eventDescription  assigned ok
			FMOD::Studio::EventInstance* pEventInstance = NULL; //create an event instance
			
			//eventInstance is a playable instance of eventDescription - multiple instances can be made from a single event
			fmodErrorCheck(pEventDescription->createInstance(&pEventInstance));
			if (pEventInstance) { //add event to implementation map
				_sgpImplementation->mEvents[strEventName] = pEventInstance;
			}
		}
	}

	//Plays an FMOD event
	void AudioEngine::playEvent(const std::string &strEventName) {
		//get event from map 
		auto tFoundit = _sgpImplementation->mEvents.find(strEventName);
		if (tFoundit == _sgpImplementation->mEvents.end()) { //if not in map, load the event
			loadEvent(strEventName);
			tFoundit = _sgpImplementation->mEvents.find(strEventName);
			if (tFoundit == _sgpImplementation->mEvents.end())
				return;
		}
		tFoundit->second->start(); //play the event
	}

	//Stops a channel
	void AudioEngine::stopChannel(int nChannelId) {
		//find channel
		auto tFoundIt = _sgpImplementation->mChannels.find(nChannelId);
		if (tFoundIt == _sgpImplementation->mChannels.end())
			return; //not found, just return

		fmodErrorCheck(tFoundIt->second->stop()); //stop the channel
	}

	//Stops an FMOD event.
	void AudioEngine::stopEvent(const std::string &strEventName, bool bImmediate) {
		//find event
		auto tFoundIt = _sgpImplementation->mEvents.find(strEventName);
		if (tFoundIt == _sgpImplementation->mEvents.end())
			return; //not found, just return

		//set stop mode
		//If IMMEDIATE, doesn't include trailing amplitude envelope or DSP effects, and simply cuts the audio.
		FMOD_STUDIO_STOP_MODE eMode;
		eMode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
		fmodErrorCheck(tFoundIt->second->stop(eMode));
	}

	//stops all channels
	void AudioEngine::stopAllChannels() {
		//iterate through all channels in our map and attempt to stop them
		for (auto channelIt = _sgpImplementation->mChannels.begin(); channelIt != _sgpImplementation->mChannels.end(); channelIt++) {
			fmodErrorCheck(channelIt->second->stop());
		}
	}

	// Nav: change the volume on all channels
	void AudioEngine::changeMasterVolume(float fVolumedB) {
		float *currVolume = new float;
		for (auto channelIt = _sgpImplementation->mChannels.begin(); channelIt != _sgpImplementation->mChannels.end(); channelIt++) {
			fmodErrorCheck(channelIt->second->getVolume(currVolume));
			float v = (*currVolume);
			fmodErrorCheck(channelIt->second->setVolume((*currVolume) + fVolumedB));
		}
	}

	//checks if a channel is currently playing a sound
	bool AudioEngine::isChannelPlaying(int nChannelId) const {
		auto tFoundIt = _sgpImplementation->mChannels.find(nChannelId);
		if (tFoundIt == _sgpImplementation->mChannels.end())
			return false; //if channel not in map, it's not playing

		bool isPlaying = false;
		tFoundIt->second->isPlaying(&isPlaying);

		return isPlaying;
	}

	//Checks if FMOD event is playing
	bool AudioEngine::isEventPlaying(const std::string &strEventName) const {
		auto tFoundIt = _sgpImplementation->mEvents.find(strEventName);
		if (tFoundIt == _sgpImplementation->mEvents.end())
			return false; //if event not in map, it's not playing

		//if found, check playback state
		FMOD_STUDIO_PLAYBACK_STATE* state = NULL;
		if (tFoundIt->second->getPlaybackState(state) == FMOD_STUDIO_PLAYBACK_PLAYING) {
			return true;
		}
		return false;
	}

	void AudioEngine::toggleMute()
	{
		_mute = !_mute;

		//get reference to master bus
		FMOD::Studio::Bus * masterBus;
		_sgpImplementation->mpStudioSystem->getBus("bus:/", &masterBus);

		masterBus->setMute(_mute);
	}

	bool AudioEngine::getMute() 
	{
		return _mute;
	}

	//The following handles FMOD event parameters
	//Event parameters allow dynamic changes to events such as pitch, length, volume etc.

	//gets the FMOD event parameter if given event and parameter name
	void AudioEngine::getEventParameter(const std::string &strEventName, const std::string &strParameterName, float* parameter) {
		auto tFoundIt = _sgpImplementation->mEvents.find(strEventName);
		if (tFoundIt == _sgpImplementation->mEvents.end())
			return;

		FMOD::Studio::ParameterInstance* pParameter = NULL;
		fmodErrorCheck(tFoundIt->second->getParameter(strParameterName.c_str(), &pParameter));
		fmodErrorCheck(pParameter->getValue(parameter));
	}


	//sets the FMOD event parameter if given event and parameter name
	void AudioEngine::setEventParameter(const std::string &strEventName, const std::string &strParameterName, float fValue) {
		auto tFoundIt = _sgpImplementation->mEvents.find(strEventName);
		if (tFoundIt == _sgpImplementation->mEvents.end())
			return;

		FMOD::Studio::ParameterInstance* pParameter = NULL;
		fmodErrorCheck(tFoundIt->second->getParameter(strParameterName.c_str(), &pParameter));
		fmodErrorCheck(pParameter->setValue(fValue));
	}

	//clean up AudioEngine
	void AudioEngine::shutdown() {
		delete _sgpImplementation;
	}

	/*
	* Utility functions for converting values.
	*/
	
	//Converts an glm vector3 to FMOD, for easy 3d audio
	FMOD_VECTOR AudioEngine::vectorToFmod(const glm::vec3& vPosition) {
		FMOD_VECTOR fVec;
		fVec.x = vPosition.x;
		fVec.y = vPosition.y;
		fVec.z = vPosition.z;
		return fVec;
	}

	//get linear volume from db
	float  AudioEngine::dbToVolume(float dB)
	{
		// 10^db/20
		return powf(10.0f, 0.05f * dB);
	}

	//get dB from linear volume
	float  AudioEngine::volumeTodB(float volume)
	{
		//log(volume) * 20
		return 20.0f * log10f(volume);
	}
}