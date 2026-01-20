#include "BassNationSpectrumLayer.hpp"

namespace avz::examples
{

BassNationSpectrumLayer::BassNationSpectrumLayer(
	int fft_size, int sample_rate, sf::Vector2u size, const avz::ColorSettings &cs, bool left)
	: fa{fft_size},
	  aa{sample_rate, fft_size},
	  spectrum{{{}, (sf::Vector2i)size}, cs},
	  is_left(left),
	  sample_rate(sample_rate)
{
	a.resize(fft_size);
	fa.set_window_func(avz::FrequencyAnalyzer::WindowFunction::Blackman);
	worker = std::thread{&BassNationSpectrumLayer::worker_loop, this};
}

BassNationSpectrumLayer::~BassNationSpectrumLayer()
{
	{
		std::lock_guard lk(mu);
		stop = true;
		has_work = false;
	}
	cv.notify_one();
	if (worker.joinable())
		worker.join();
}

void BassNationSpectrumLayer::compute(std::span<const float> audio_buffer)
{
	const int channel = is_left ? 0 : 1;
	avz::util::extract_channel(a, audio_buffer.first(fa.get_fft_size() * 2), 2, channel);
	aa.execute_fft(fa, a);
	const auto amps = aa.compute_amplitudes(fa);
	avz::util::resample_spectrum(s, amps, sample_rate, fa.get_fft_size(), 20.0f, 135.0f, ip);
	spectrum.update(s);
}

std::future<void> BassNationSpectrumLayer::trigger_work(std::span<const float> audio_buffer)
{
	std::promise<void> p;
	auto fut = p.get_future();
	{
		std::lock_guard lk(mu);
		work_audio = audio_buffer;
		work_promise = std::move(p);
		has_work = true;
	}
	cv.notify_one();
	return fut;
}

void BassNationSpectrumLayer::configure_spectrum(bool prev, sf::Vector2u size)
{
	spectrum.set_bar_width(1);
	spectrum.set_bar_spacing(0);
	spectrum.set_multiplier(6);
	spectrum.set_backwards(prev);
	spectrum.update_bar_colors();
	s.resize(spectrum.get_bar_count());
}

void BassNationSpectrumLayer::worker_loop()
{
	while (true)
	{
		std::unique_lock lk(mu);
		cv.wait(lk, [this] { return has_work || stop; });
		if (stop)
			break;

		// copy work params
		auto audio = work_audio;
		auto prom = std::move(work_promise);
		has_work = false;
		lk.unlock();

		try
		{
			compute(audio);
			prom.set_value();
		}
		catch (...)
		{
			try
			{
				prom.set_exception(std::current_exception());
			}
			catch (...)
			{
			}
		}
	}
}

} // namespace avz::examples
