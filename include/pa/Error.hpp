#pragma once

#include <portaudio.h>
#include <stdexcept>

namespace pa
{
	// PortAudio error struct. Stores the `PaError` code and constructs a message using `Pa_GetErrorText`.
	struct Error : std::runtime_error
	{
		const PaError code;
		Error(const std::string &func, const PaError code)
			: std::runtime_error(func + ": " + Pa_GetErrorText(code)),
			  code(code) {}
	};
}
