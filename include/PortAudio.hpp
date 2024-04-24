#pragma once

#include <portaudio.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace Pa
{
	struct Error : std::runtime_error
	{
		const PaError code;
		Error(const PaError code) : std::runtime_error(std::string("portaudio: ") + Pa_GetErrorText(code)), code(code) {}
	};

	struct PortAudio
	{
		PortAudio()
		{
			if (const auto rc = Pa_Initialize())
				throw Error(rc);
		}

		~PortAudio()
		{
			if (const auto rc = Pa_Terminate())
				std::cerr << Pa_GetErrorText(rc) << '\n';
		}

		PortAudio(const PortAudio &) = delete;
		PortAudio &operator=(const PortAudio &) = delete;
		PortAudio(PortAudio &&) = delete;
		PortAudio &operator=(PortAudio &&) = delete;
	};

	using Float32 = float;
	using Int32 = int32_t;
	// clang-format off
	struct Int24 { uint8_t bytes[3]; };
	// clang-format on
	using Int16 = int16_t;
	using Int8 = int8_t;
	using UInt8 = uint8_t;

	template <typename _Tp>
	class Stream
	{
		PaStream *stream;
		int numOutputChannels;
		PaSampleFormat sampleFormat;

	public:
		Stream(int numInputChannels, int numOutputChannels, double sampleRate, unsigned long framesPerBuffer, bool interleaved = true, PaStreamCallback *streamCallback = NULL, void *userData = NULL)
			: numOutputChannels(numOutputChannels)
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
				throw Error(rc);
			if (const auto rc = Pa_StartStream(stream))
				throw Error(rc);
		}

		~Stream()
		{
			if (const auto rc = Pa_StopStream(stream))
				std::cerr << Pa_GetErrorText(rc) << '\n';
			if (const auto rc = Pa_CloseStream(stream))
				std::cerr << Pa_GetErrorText(rc) << '\n';
		}

		Stream(const Stream &) = delete;
		Stream &operator=(const Stream &) = delete;
		Stream(Stream &&) = delete;
		Stream &operator=(Stream &&) = delete;

		// /**
		//  * @throws `std::runtime_error` if this stream's sample format is non-interleaved
		//  */
		// void operator<<(const std::vector<_Tp> &buffer)
		// {
		// 	if (sampleFormat & paNonInterleaved)
		// 		throw std::runtime_error("sample format is non-interleaved!");
		// 	_write(buffer.data(), buffer.size() / numOutputChannels);
		// }

		// void write(const std::vector<_Tp> &buffer, const size_t frames)
		// {
		// 	if (sampleFormat & paNonInterleaved)
		// 		throw std::runtime_error("sample format is non-interleaved!");
		// 	_write(buffer.data(), frames);
		// }

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
				throw Error(rc);
		}
	};
};
