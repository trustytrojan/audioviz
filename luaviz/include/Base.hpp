#pragma once

#include <audioviz/Base.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>

#define capture_time(label, code)            \
	if (timing_text_enabled())               \
	{                                        \
		sf::Clock _clock;                    \
		code;                                \
		capture_elapsed_time(label, _clock); \
	}                                        \
	else                                     \
		code;

namespace luaviz
{

struct Base : public audioviz::Base
{
	audioviz::Media &media;

	void perform_fft(audioviz::fft::FrequencyAnalyzer &fa, audioviz::fft::AudioAnalyzer &aa)
	{
		capture_time("fft", aa.analyze(fa, media.audio_buffer().data(), true));
	}

	Base(const sf::Vector2u size, audioviz::Media &media)
		: audioviz::Base{size},
		  media{media}
	{
	}
};

} // namespace luaviz
