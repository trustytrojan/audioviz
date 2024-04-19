#pragma once

#include <iostream>
#include <stdexcept>
#include <portaudio.h>

struct PortAudio
{
	class Error : public std::runtime_error
	{
		friend class PortAudio;
		Error(const PaError err) : std::runtime_error(std::string("portaudio: ") + Pa_GetErrorText(err)), err(err) {}

	public:
		const PaError err;
	};

	class Stream
	{
		friend class PortAudio;
		PaStream *stream;

		Stream(int numInputChannels, int numOutputChannels, PaSampleFormat sampleFormat, double sampleRate, unsigned long framesPerBuffer, PaStreamCallback *streamCallback, void *userData)
		{
			PaError err;
			if ((err = Pa_OpenDefaultStream(&stream, numInputChannels, numOutputChannels, sampleFormat, sampleRate, framesPerBuffer, streamCallback, userData)))
				throw Error(err);
			if ((err = Pa_StartStream(stream)))
				throw Error(err);
		}

	public:
		~Stream()
		{
			PaError err;
			if ((err = Pa_StopStream(stream)))
				std::cerr << Pa_GetErrorText(err) << '\n';
			if ((err = Pa_CloseStream(stream)))
				std::cerr << Pa_GetErrorText(err) << '\n';
		}

		Stream(const Stream &) = delete;
		Stream &operator=(const Stream &) = delete;
		Stream(Stream &&) = delete;
		Stream &operator=(Stream &&) = delete;

		void reopen(int numInputChannels, int numOutputChannels, PaSampleFormat sampleFormat, double sampleRate, unsigned long framesPerBuffer, PaStreamCallback *streamCallback = NULL, void *userData = NULL)
		{
			PaError err;
			if ((err = Pa_StopStream(stream)))
				throw Error(err);
			if ((err = Pa_CloseStream(stream)))
				throw Error(err);
			if ((err = Pa_OpenDefaultStream(&stream, numInputChannels, numOutputChannels, sampleFormat, sampleRate, framesPerBuffer, streamCallback, userData)))
				throw Error(err);
			if ((err = Pa_StartStream(stream)))
				throw Error(err);
		}

		void write(const float *const buffer, const size_t n_frames)
		{
			PaError err;
			if ((err = Pa_WriteStream(stream, buffer, n_frames)))
				throw Error(err);
		}
	};

	PortAudio()
	{
		PaError err;
		if ((err = Pa_Initialize()))
			throw Error(err);
	}

	~PortAudio()
	{
		PaError err;
		if ((err = Pa_Terminate()))
			std::cerr << Pa_GetErrorText(err) << '\n';
	}

	PortAudio(const PortAudio &) = delete;
	PortAudio &operator=(const PortAudio &) = delete;
	PortAudio(PortAudio &&) = delete;
	PortAudio &operator=(PortAudio &&) = delete;

	Stream stream(int numInputChannels, int numOutputChannels, PaSampleFormat sampleFormat, double sampleRate, unsigned long framesPerBuffer, PaStreamCallback *streamCallback = NULL, void *userData = NULL)
	{
		return Stream(numInputChannels, numOutputChannels, sampleFormat, sampleRate, framesPerBuffer, streamCallback, userData);
	}
};