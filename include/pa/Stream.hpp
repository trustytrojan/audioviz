#pragma once

#include <portaudio.h>
#include "Error.hpp"
#include "Types.hpp"

namespace pa
{
	class Stream
	{
		PaStream *_stream;

	public:
		Stream(int numInputChannels, int numOutputChannels, PaSampleFormat sampleFormat, double sampleRate, unsigned long framesPerBuffer, bool interleaved = true, PaStreamCallback *streamCallback = NULL, void *userData = NULL)
		{
			if (const auto rc = Pa_OpenDefaultStream(&_stream, numInputChannels, numOutputChannels, sampleFormat, sampleRate, framesPerBuffer, streamCallback, userData))
				throw Error("Pa_OpenDefaultStream", rc);
			if (const auto rc = Pa_StartStream(_stream))
				throw Error("Pa_StartStream", rc);
		}

		~Stream()
		{
			if (!_stream)
				return;
			if (const auto rc = Pa_StopStream(_stream))
				std::cerr << "Pa_StopStream: " << Pa_GetErrorText(rc) << '\n';
			if (const auto rc = Pa_CloseStream(_stream))
				std::cerr << "Pa_CloseStream: " << Pa_GetErrorText(rc) << '\n';
		}

		// Delete copy constructor
		Stream(const Stream &) = delete;

		// Delete copy assignment operator
		Stream &operator=(const Stream &) = delete;

		// Move constructor
		Stream(Stream &&other)
		{
			// close this stream
			this->~Stream();

			// take `other`'s stream, making sure its destructor call won't error
			_stream = other._stream;
			other._stream = nullptr;
		}

		// Move assignment operator
		Stream &operator=(Stream &&other)
		{
			// close this stream
			this->~Stream();

			// take `other`'s stream, making sure its destructor call won't error
			_stream = other._stream;
			other._stream = nullptr;

			return *this;
		}

		void write(const void *const buffer, const size_t frames)
		{
			if (const auto rc = Pa_WriteStream(_stream, buffer, frames))
				throw Error("Pa_WriteStream", rc);
		}
	};
}