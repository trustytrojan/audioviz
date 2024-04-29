#pragma once

#include <portaudio.h>
#include "Error.hpp"
#include "Types.hpp"

namespace pa
{
	template <typename _Tp>
	class Stream
	{
		PaStream *stream;
		PaSampleFormat sampleFormat;

	public:
		Stream(int numInputChannels, int numOutputChannels, double sampleRate, unsigned long framesPerBuffer, bool interleaved = true, PaStreamCallback *streamCallback = NULL, void *userData = NULL)
		{
			if constexpr (std::is_same_v<_Tp, Float32>)
				sampleFormat = paFloat32;
			else if constexpr (std::is_same_v<_Tp, Int32>)
				sampleFormat = paInt32;
			else if constexpr (std::is_same_v<_Tp, Int24>)
				sampleFormat = paInt24;
			else if constexpr (std::is_same_v<_Tp, Int16>)
				sampleFormat = paInt16;
			else if constexpr (std::is_same_v<_Tp, Int8>)
				sampleFormat = paInt8;
			else if constexpr (std::is_same_v<_Tp, UInt8>)
				sampleFormat = paUInt8;

			if (!interleaved)
				sampleFormat |= paNonInterleaved;

			if (const auto rc = Pa_OpenDefaultStream(&stream, numInputChannels, numOutputChannels, sampleFormat, sampleRate, framesPerBuffer, streamCallback, userData))
				throw Error("Pa_OpenDefaultStream", rc);
			if (const auto rc = Pa_StartStream(stream))
				throw Error("Pa_StartStream", rc);
		}

		~Stream()
		{
			if (const auto rc = Pa_StopStream(stream))
				std::cerr << "Pa_StopStream: " << Pa_GetErrorText(rc) << '\n';
			if (const auto rc = Pa_CloseStream(stream))
				std::cerr << "Pa_CloseStream: " << Pa_GetErrorText(rc) << '\n';
		}

		Stream(const Stream &) = delete;
		Stream &operator=(const Stream &) = delete;
		Stream(Stream &&) = delete;
		Stream &operator=(Stream &&) = delete;

		void write(const _Tp *const buffer, const size_t frames)
		{
			if (sampleFormat & paNonInterleaved)
				throw std::runtime_error("sample format is non-interleaved!");
			_write(buffer, frames);
		}

	private:
		void _write(const void *const buffer, const size_t frames)
		{
			if (const auto rc = Pa_WriteStream(stream, buffer, frames))
				throw Error("Pa_WriteStream", rc);
		}
	};
}