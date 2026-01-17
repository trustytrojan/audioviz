#include <audioviz/Base.hpp>
#include <audioviz/Player.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fft/Interpolator.hpp>
#include <audioviz/fx/Polar.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/util.hpp>

#include <SFML/Graphics.hpp>
#include <condition_variable>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <print>
#include <thread>

constexpr float audio_duration_sec = 0.25;

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
		// a.resize(fa.get_fft_size());
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
	void start_worker()
	{
		worker = std::thread([this]() { worker_loop(); });
	}

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

struct OldBassNation : audioviz::Base
{
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz = media.audio_sample_rate();
	int num_channels{media.audio_channels()};
	const int max_fft_size = audio_duration_sec * sample_rate_hz;

	std::vector<std::unique_ptr<SpectrumLayer>> spectrums;
	audioviz::ColorSettings cs;
	std::vector<std::future<void>> futures;

	audioviz::fx::Polar polar_left{(sf::Vector2f)size, size.y * 0.25, size.y * 0.5, M_PI / 2, M_PI},
		polar_right{polar_left};

	OldBassNation(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

OldBassNation::OldBassNation(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  media{media_url}
{
#ifdef __linux__
	enable_profiler();
	set_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	std::println("max_fft_size={} sample_rate_hz={}", max_fft_size, sample_rate_hz);

	cs.set_mode(audioviz::ColorSettings::Mode::SOLID);

	polar_right.angle_start = -M_PI / 2;

	static const std::array<sf::Color, 9> colors{
		sf::Color::Green,
		sf::Color::Cyan,
		sf::Color::Blue,
		sf::Color{146, 29, 255}, // purple
		sf::Color::Magenta,
		sf::Color::Red,
		sf::Color{255, 165, 0}, // orange
		sf::Color::Yellow,
		sf::Color::White //
	};

	const auto delta_duration = 0.015f;
	const auto max_duration_diff = (colors.size() - 1) * delta_duration;

	auto &spectrum_layer = emplace_layer<audioviz::PostProcessLayer>("spectrum", size);

	for (int i = 0; i < colors.size(); ++i)
	{
		float duration_diff = max_duration_diff - i * delta_duration;
		const auto new_duration_sec = audio_duration_sec - duration_diff;
		const int new_fft_size = new_duration_sec * sample_rate_hz;

		cs.set_solid_color(colors[i]);

		auto &left_layer =
			*spectrums.emplace_back(std::make_unique<SpectrumLayer>(new_fft_size, sample_rate_hz, size, cs, true));
		left_layer.configure_spectrum(false, size);

		auto &right_layer =
			*spectrums.emplace_back(std::make_unique<SpectrumLayer>(new_fft_size, sample_rate_hz, size, cs, false));
		right_layer.configure_spectrum(true, size);

		spectrum_layer.add_draw({left_layer.spectrum, &polar_left});
		spectrum_layer.add_draw({right_layer.spectrum, &polar_right});
	}

	futures.resize(spectrums.size());
}

void OldBassNation::update(const std::span<const float> audio_buffer)
{
	std::ranges::transform(spectrums, futures.begin(), std::bind_back(&SpectrumLayer::trigger_work, audio_buffer));

	// wait for all compute tasks
	std::ranges::for_each(futures, &std::future<void>::wait);
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{std::stoul(argv[1]), std::stoul(argv[2])};
	OldBassNation viz{size, argv[3]};
	audioviz::Player{viz, viz.media, 60, viz.max_fft_size}.start_in_window(argv[0]);
}
