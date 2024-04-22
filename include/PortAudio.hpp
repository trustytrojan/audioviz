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

	PortAudio()
	{
		if (const auto err = Pa_Initialize())
			throw Error(err);
	}

	~PortAudio()
	{
		if (const auto err = Pa_Terminate())
			std::cerr << Pa_GetErrorText(err) << '\n';
	}

	PortAudio(const PortAudio &) = delete;
	PortAudio &operator=(const PortAudio &) = delete;
	PortAudio(PortAudio &&) = delete;
	PortAudio &operator=(PortAudio &&) = delete;

	class Stream
	{
		PaStream *stream;

	public:
		Stream(int numInputChannels, int numOutputChannels, PaSampleFormat sampleFormat, double sampleRate, unsigned long framesPerBuffer, PaStreamCallback *streamCallback = NULL, void *userData = NULL)
		{
			if (const auto err = Pa_OpenDefaultStream(&stream, numInputChannels, numOutputChannels, sampleFormat, sampleRate, framesPerBuffer, streamCallback, userData))
				throw Error(err);
			if (const auto err = Pa_StartStream(stream))
				throw Error(err);
		}

		~Stream()
		{
			if (const auto err = Pa_StopStream(stream))
				std::cerr << Pa_GetErrorText(err) << '\n';
			if (const auto err = Pa_CloseStream(stream))
				std::cerr << Pa_GetErrorText(err) << '\n';
		}

		Stream(const Stream &) = delete;
		Stream &operator=(const Stream &) = delete;
		Stream(Stream &&) = delete;
		Stream &operator=(Stream &&) = delete;

		void reopen(int numInputChannels, int numOutputChannels, PaSampleFormat sampleFormat, double sampleRate, unsigned long framesPerBuffer, PaStreamCallback *streamCallback = NULL, void *userData = NULL)
		{
			if (const auto err = Pa_StopStream(stream))
				throw Error(err);
			if (const auto err = Pa_CloseStream(stream))
				throw Error(err);
			if (const auto err = Pa_OpenDefaultStream(&stream, numInputChannels, numOutputChannels, sampleFormat, sampleRate, framesPerBuffer, streamCallback, userData))
				throw Error(err);
			if (const auto err = Pa_StartStream(stream))
				throw Error(err);
		}

		void write(const float *const buffer, const size_t n_frames)
		{
			if (const auto err = Pa_WriteStream(stream, buffer, n_frames))
				throw Error(err);
		}
	};
};
