#pragma once

#include <portaudio.h>
#include <iostream>
#include "Error.hpp"

namespace pa
{
	// library initialization and termination
	// you must own an instance of `PortAudio` before doing anything else with the library
	struct PortAudio
	{
		PortAudio()
		{
			if (const auto rc = Pa_Initialize())
				throw Error("Pa_Initialize", rc);
		}

		~PortAudio()
		{
			if (const auto rc = Pa_Terminate())
				std::cerr << "Pa_Terminate: " << Pa_GetErrorText(rc) << '\n';
		}

		PortAudio(const PortAudio &) = delete;
		PortAudio &operator=(const PortAudio &) = delete;
		PortAudio(PortAudio &&) = delete;
		PortAudio &operator=(PortAudio &&) = delete;
	};
};
