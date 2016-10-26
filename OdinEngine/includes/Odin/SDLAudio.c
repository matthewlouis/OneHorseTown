/*
* Modified Simple-SDL2-Audio to multithread playback
*
*/

/*
 * Simple-SDL2-Audio
 *
 * Copyright 2016 Jake Besworth
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL\SDL.h>
#include "SDLAudio.h"

	/*
	 * Native WAVE format
	 *
	 * On some GNU/Linux you can identify a files properties using:
	 *      mplayer -identify music.wav
	 *
	 * On some GNU/Linux to convert any music to this or another specified format use:
	 *      ffmpeg -i in.mp3 -acodec pcm_s16le -ac 2 -ar 48000 out.wav
	 */
	 /* SDL_AudioFormat of files, such as s16 little endian */
#define AUDIO_FORMAT AUDIO_S16LSB

/* Frequency of the file */
#define AUDIO_FREQUENCY 48000;

/* 1 mono, 2 stereo, 4 quad, 6 (5.1) */
#define AUDIO_CHANNELS 2;

/* Specifies a unit of audio data to be used at a time. Must be a power of 2 */
#define AUDIO_SAMPLES 4096;

/*
 * Thread structure for passing data to callback thread
 */
	typedef struct threadData
	{
		void * userdata;
		uint8_t * stream;
		int len;
	}ThreadData;


/*
 * Queue structure for all loaded sounds
 *
 */
	typedef struct sound
	{
		uint32_t length;
		uint32_t lengthTrue;
		uint8_t * bufferTrue;
		uint8_t * buffer;
		uint8_t volume;

		SDL_AudioSpec audio;

		struct sound * next;
	} Sound;

	/*
	 * For multithreading
	 */
	static int playThread(void *);

	static SDL_mutex * sgpMutex;

	/*
	 * Definition for the game global sound device
	 *
	 */
	typedef struct privateAudioDevice
	{
		SDL_AudioDeviceID device;
		SDL_AudioSpec want;
		uint8_t audioEnabled;
	} PrivateAudioDevice;

	/*
	 * Add a sound to the end of the queue
	 *
	 * @param root      Root of queue
	 * @param new       New Sound to add
	 *
	 */
	static void addSound(Sound * root, Sound * new);

	/*
	 * Frees as many chained Sounds as given
	 *
	 * @param sound     Chain of sounds to free
	 *
	 */
	static void freeSound(Sound * sound);

	/*
	 * Create a Sound object
	 *
	 * @param filename      Filename for the WAVE file to load
	 * @param loop          Loop 0, ends after playing, 1 refreshes
	 * @param volume        Volume, read playSound()
	 *
	 * @return returns a new Sound or NULL on failure
	 *
	 */
	static Sound * createSound(const char * filename, int volume);

	/*
	 * Audio callback function for OpenAudioDevice
	 *
	 * @param userdata      Points to linked list of sounds to play, first being a placeholder
	 * @param stream        Stream to mix sound into
	 * @param len           Length of sound to play
	 *
	 */
	static inline void audioCallback(void * userdata, uint8_t * stream, int len);

	static PrivateAudioDevice * gDevice;


	void playSound(const char * filename, int volume)
	{
		Sound * new;

		if (!gDevice->audioEnabled)
		{
			return;
		}

		new = createSound(filename, volume);

		SDL_LockAudioDevice(gDevice->device);
		addSound((Sound *)(gDevice->want).userdata, new);

		SDL_UnlockAudioDevice(gDevice->device);
	}

	void initAudio(void)
	{
		Sound * global;
		gDevice = calloc(1, sizeof(PrivateAudioDevice));

		if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO))
		{
			fprintf(stderr, "[%s: %d]Error: SDL_INIT_AUDIO not initialized\n", __FILE__, __LINE__);
			gDevice->audioEnabled = 0;
			return;
		}
		else
		{
			gDevice->audioEnabled = 1;
		}

		if (gDevice == NULL)
		{
			fprintf(stderr, "[%s: %d]Fatal Error: Memory c-allocation error\n", __FILE__, __LINE__);
			return;
		}

		SDL_memset(&(gDevice->want), 0, sizeof(gDevice->want));

		(gDevice->want).freq = AUDIO_FREQUENCY;
		(gDevice->want).format = AUDIO_FORMAT;
		(gDevice->want).channels = AUDIO_CHANNELS;
		(gDevice->want).samples = AUDIO_SAMPLES;
		(gDevice->want).callback = audioCallback;
		(gDevice->want).userdata = calloc(1, sizeof(Sound));

		global = (gDevice->want).userdata;

		if (global == NULL)
		{
			fprintf(stderr, "[%s: %d]Error: Memory allocation error\n", __FILE__, __LINE__);
			return;
		}

		global->buffer = NULL;
		global->next = NULL;

		/* want.userdata = new; */
		if ((gDevice->device = SDL_OpenAudioDevice(NULL, 0, &(gDevice->want), NULL, SDL_AUDIO_ALLOW_ANY_CHANGE)) == 0)
		{
			fprintf(stderr, "[%s: %d]Warning: failed to open audio device: %s\n", __FILE__, __LINE__, SDL_GetError());
		}
		else
		{
			/* Unpause active audio stream */
			SDL_PauseAudioDevice(gDevice->device, 0);
		}

		sgpMutex = SDL_CreateMutex();
	}

	void endAudio(void)
	{
		if (gDevice->audioEnabled)
		{
			SDL_PauseAudioDevice(gDevice->device, 1);

			freeSound((Sound *)(gDevice->want).userdata);

			/* Close down audio */
			SDL_CloseAudioDevice(gDevice->device);
		}

		free(gDevice);
		SDL_DestroyMutex(sgpMutex);
	}

	static Sound * createSound(const char * filename, int volume)
	{
		Sound * new = calloc(1, sizeof(Sound));

		if (new == NULL)
		{
			fprintf(stderr, "[%s: %d]Error: Memory allocation error\n", __FILE__, __LINE__);
			return NULL;
		}

		new->next = NULL;
		new->volume = volume;

		if (SDL_LoadWAV(filename, &(new->audio), &(new->bufferTrue), &(new->lengthTrue)) == NULL)
		{
			fprintf(stderr, "[%s: %d]Warning: failed to open wave file: %s err: %s\n", __FILE__, __LINE__, filename, SDL_GetError());
			free(new);
			return NULL;
		}

		new->buffer = new->bufferTrue;
		new->length = new->lengthTrue;
		(new->audio).callback = NULL;
		(new->audio).userdata = NULL;

		return new;
	}

	static inline void audioCallback(void * userdata, uint8_t * stream, int len)
	{
		/* Silence the main buffer */
		SDL_memset(stream, 0, len);

		Sound * s = userdata;

		//try to lock mutex, if not don't block just return because an audio thread is currently running
		if (SDL_TryLockMutex(sgpMutex) != 0 || s->next == NULL) {

			return; //in use
		}

		ThreadData * tData = calloc(1, sizeof(ThreadData));
		tData->userdata = userdata;
		tData->stream = stream;
		tData->len = len;

		SDL_Thread * thread = SDL_CreateThread(playThread, "playThread", tData);
	}

	static void addSound(Sound * root, Sound * new)
	{
		if (root == NULL)
		{
			return;
		}

		while (root->next != NULL)
		{
			root = root->next;
		}

		root->next = new;
	}

	static void freeSound(Sound * sound)
	{
		Sound * temp;

		while (sound != NULL)
		{
			SDL_FreeWAV(sound->bufferTrue);

			temp = sound;
			sound = sound->next;

			free(temp);
		}
	}

	static int playThread(void * data) {
		ThreadData * tData = data;
		
		Sound * sound = (Sound *)tData->userdata;
		Sound * previous = sound;
		int tempLength;

		/* First one is place holder */
		sound = sound->next;

		while (sound != NULL)
		{
			if (sound->length > 0)
			{

				tempLength = ((uint32_t)tData->len > sound->length) ? sound->length : (uint32_t)tData->len;

				SDL_MixAudioFormat(tData->stream, sound->buffer, AUDIO_FORMAT, tempLength, sound->volume);

				sound->buffer += tempLength;
				sound->length -= tempLength;

				previous = sound;
				sound = sound->next;
			}
			else
			{
				previous->next = sound->next;
				SDL_FreeWAV(sound->bufferTrue);
				free(sound);

				sound = previous->next;
			}
		}
		printf("\nThread finished");
		SDL_UnlockMutex(sgpMutex);
	}