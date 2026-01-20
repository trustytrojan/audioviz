#pragma once

#include <avz/SpectrumDrawable.hpp>
#include <avz/fft/AudioAnalyzer.hpp>
#include <avz/fft/FrequencyAnalyzer.hpp>
#include <avz/fft/Interpolator.hpp>
#include <avz/util.hpp>

#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>

struct SpectrumLayer
{
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa;
	audioviz::Interpolator ip;
	std::vector<float, aligned_allocator<float>> s, a;
	audioviz::SpectrumDrawable spectrum;
	bool is_left;
	int sample_rate;

	SpectrumLayer(int fft_size, int sample_rate, sf::Vector2u size, const audioviz::ColorSettings &cs, bool left)
		: fa{fft_size},
		  aa{sample_rate, fft_size},
		  spectrum{{{}, (sf::Vector2i)size}, cs},
		  is_left(left),
		  sample_rate(sample_rate)
	{
		a.resize(fft_size);
		fa.set_window_func(audioviz::FrequencyAnalyzer::WindowFunction::Blackman);
		start_worker();
	}

	~SpectrumLayer()
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

	void compute(std::span<const float> audio_buffer)
	{
		const int channel = is_left ? 0 : 1;
		audioviz::util::extract_channel(a, audio_buffer.first(fa.get_fft_size() * 2), 2, channel);
		aa.execute_fft(fa, a);
		const auto amps = aa.compute_amplitudes(fa);
		audioviz::util::resample_spectrum(s, amps, sample_rate, fa.get_fft_size(), 20.0f, 135.0f, ip);
		spectrum.update(s);
	}

	std::future<void> trigger_work(std::span<const float> audio_buffer)
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

	// configure spectrum drawable and resize internal buffers
	void configure_spectrum(bool prev, sf::Vector2u size)
	{
		spectrum.set_bar_width(1);
		spectrum.set_bar_spacing(0);
		spectrum.set_multiplier(6);
		spectrum.set_backwards(prev);
		spectrum.update_bar_colors();
		s.resize(spectrum.get_bar_count());
	}

private:
	void start_worker() { worker = std::thread{&SpectrumLayer::worker_loop, this}; }

	void worker_loop()
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

	// worker state
	std::thread worker;
	std::mutex mu;
	std::condition_variable cv;
	bool has_work{false};
	bool stop{false};
	std::span<const float> work_audio;
	std::promise<void> work_promise;
};
