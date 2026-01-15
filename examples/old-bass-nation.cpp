#include <audioviz/Base.hpp>
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

#define capture_time(label, code)            \
	if (timing_text_enabled())               \
	{                                        \
		sf::Clock _clock;                    \
		code;                                \
		capture_elapsed_time(label, _clock); \
	}                                        \
	else                                     \
		code;

constexpr float audio_duration_sec = 0.25;

struct LayerData
{
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa;
	audioviz::Interpolator ip;
	std::vector<float, aligned_allocator<float>> s, a;
	audioviz::SpectrumDrawable spectrum;
	bool is_left;

	LayerData(int fft_size, int sample_rate, sf::Vector2u size, const audioviz::ColorSettings &cs, bool left)
		: fa{fft_size},
		  aa{sample_rate, fft_size},
		  spectrum{{{}, (sf::Vector2i)size}, cs},
		  is_left(left)
	{
		a.resize(fft_size);
		fa.set_window_func(audioviz::FrequencyAnalyzer::WindowFunction::Blackman);
		start_worker();
	}

	~LayerData()
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

	void compute(std::span<const float> audio_buffer, int num_channels, int max_fft_size, int sample_rate_hz)
	{
		const int channel = is_left ? 0 : 1;
		audioviz::util::strided_copy(a, audio_buffer.first(max_fft_size * num_channels), num_channels, channel);
		aa.execute_fft(fa, a);
		const auto amps = aa.compute_amplitudes(fa);
		audioviz::util::resample_spectrum(s, amps, sample_rate_hz, fa.get_fft_size(), 20.0f, 135.0f, ip);
		spectrum.update(s);
	}

	std::future<void>
	trigger_work(std::span<const float> audio_buffer, int num_channels, int max_fft_size, int sample_rate_hz)
	{
		std::promise<void> p;
		auto fut = p.get_future();
		{
			std::lock_guard lk(mu);
			work_audio = audio_buffer;
			work_num_channels = num_channels;
			work_max_fft_size = max_fft_size;
			work_sample_rate_hz = sample_rate_hz;
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

	// draw this layer into the given render target using polar parameters
	void draw(sf::RenderTarget &rt, const sf::RenderStates &rs, sf::Vector2u size)
	{
		float angle = is_left ? M_PI / 2 : -M_PI / 2;
		audioviz::fx::Polar::setParameters((sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, angle, M_PI);
		rt.draw(spectrum, rs);
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
			int nc = work_num_channels;
			int maxfs = work_max_fft_size;
			int sr = work_sample_rate_hz;
			auto prom = std::move(work_promise);
			has_work = false;
			lk.unlock();

			try
			{
				compute(audio, nc, maxfs, sr);
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
	int work_num_channels{};
	int work_max_fft_size{};
	int work_sample_rate_hz{};
	std::promise<void> work_promise;
};

struct OldBassNation : audioviz::Base
{
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz = media.audio_sample_rate();
	int num_channels{media.audio_channels()};
	const int max_fft_size = audio_duration_sec * sample_rate_hz;

	std::vector<std::unique_ptr<LayerData>> layers;
	audioviz::ColorSettings cs;
	std::span<const float> audio_buffer;

	OldBassNation(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

OldBassNation::OldBassNation(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  media{media_url}
{
#ifdef __linux__
	set_timing_text_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	std::println("max_fft_size={} sample_rate_hz={}", max_fft_size, sample_rate_hz);

	cs.set_mode(audioviz::ColorSettings::Mode::SOLID);

	set_audio_frames_needed(max_fft_size);

	sf::RenderStates polar_rs{&audioviz::fx::Polar::getShader()};

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

	layers.reserve(colors.size() * 2);
	for (int i = 0; i < colors.size(); ++i)
	{
		float duration_diff = max_duration_diff - i * delta_duration;
		const auto new_duration_sec = audio_duration_sec - duration_diff;
		const int new_fft_size = new_duration_sec * sample_rate_hz;

		cs.set_solid_color(colors[i]);

		auto &left_layer =
			*layers.emplace_back(std::make_unique<LayerData>(new_fft_size, sample_rate_hz, size, cs, true));
		left_layer.configure_spectrum(false, size);

		auto &right_layer =
			*layers.emplace_back(std::make_unique<LayerData>(new_fft_size, sample_rate_hz, size, cs, false));
		right_layer.configure_spectrum(true, size);
	}

	std::vector<std::future<void>> futures(layers.size());

	add_layer("spectrum")
		.set_orig_cb(
			[&, polar_rs](auto &orig_rt) mutable
			{
				orig_rt.clear();

				std::ranges::transform(
					layers,
					futures.begin(),
					std::bind_back(&LayerData::trigger_work, audio_buffer, num_channels, max_fft_size, sample_rate_hz));

				// wait for all compute tasks
				std::ranges::for_each(futures, &std::future<void>::wait);

				for (auto &lp : layers)
					lp->draw(orig_rt, polar_rs, size);

				orig_rt.display();
			});

	start_in_window(media, "old-bass-nation");
	// encode(media, std::string{"old-bass-nation-"} + media.title() + ".mp4", "h264_vaapi");
}

void OldBassNation::update(const std::span<const float> audio_buffer)
{
	this->audio_buffer = audio_buffer;
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
}
