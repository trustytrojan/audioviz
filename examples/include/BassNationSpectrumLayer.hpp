#pragma once

#include <avz/analysis.hpp>
#include <avz/gfx.hpp>
#include <avz/media.hpp>

#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>

namespace avz::examples
{

struct BassNationSpectrumLayer
{
	avz::FrequencyAnalyzer fa;
	avz::AudioAnalyzer aa;
	avz::Interpolator ip;
	std::vector<float> s, a;
	avz::SpectrumDrawable spectrum;
	bool is_left;
	int sample_rate;

	BassNationSpectrumLayer(int fft_size, int sample_rate, sf::Vector2u size, const avz::ColorSettings &cs, bool left);
	~BassNationSpectrumLayer();

	void compute(std::span<const float> audio_buffer);
	std::future<void> trigger_work(std::span<const float> audio_buffer);
	void configure_spectrum(bool prev, sf::Vector2u size);

private:
	void worker_loop();

	// worker state
	std::thread worker;
	std::mutex mu;
	std::condition_variable cv;
	bool has_work{false};
	bool stop{false};
	std::span<const float> work_audio;
	std::promise<void> work_promise;
};

} // namespace avz::examples
