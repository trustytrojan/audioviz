#include "ttviz.hpp"
#include <audioviz/fx/Blur.hpp>
#include <audioviz/fx/Mult.hpp>
#include <audioviz/util.hpp>
#include <stdexcept>

ttviz::ttviz(const sf::Vector2u size, audioviz::Media &media, const int fft_size, const int antialiasing)
	: Base{size},
	  media{media},
	  fa{fft_size},
	  sa{sample_rate_hz, fft_size}
{
	// Check for stereo audio
	if (media.audio_channels() != 2)
		throw std::runtime_error{
			"ttviz requires stereo (2-channel) audio; got " + std::to_string(media.audio_channels()) + " channel(s)"};

	// background
	if (media.attached_pic() || media.has_video_stream())
	{
		auto &bg_layer = emplace_layer<audioviz::PostProcessLayer>("bg", size);

		if (media.has_video_stream())
			bg_txr = sf::Texture{size};
		else if (media.attached_pic())
			set_background(*media.attached_pic());

		bg_layer.add_draw({bg_spr});
	}

	// particles
	auto &particles_layer = emplace_layer<audioviz::PostProcessLayer>("particles", size);
	particles_layer.add_draw({ps});
	particles_layer.set_fx_cb(
		[](auto &orig_rt, auto &fx_rt, auto &target)
		{
			target.draw(fx_rt.sprite(), sf::BlendAdd);
			target.draw(orig_rt.sprite(), sf::BlendAdd);
		});

	// spectrum
	ss.set_left_backwards(true);
	auto &spectrum_layer = emplace_layer<audioviz::PostProcessLayer>("spectrum", size);
	spectrum_layer.add_draw({ss});
	spectrum_layer.set_fx_cb(
		[this](auto &orig_rt, auto &fx_rt, auto &target)
		{
			if (spectrum_bm)
			{
				target.draw(fx_rt.sprite(), *spectrum_bm);
				target.draw(orig_rt.sprite(), *spectrum_bm);
			}
			else
			{
				target.draw(fx_rt.sprite(), audioviz::util::GreatAmazingBlendMode);
				target.draw(orig_rt.sprite());
			}
		});

	// metadata
	title_text.setStyle(sf::Text::Bold | sf::Text::Italic);
	title_text.setCharacterSize(24);
	title_text.setFillColor({255, 255, 255, 150});

	artist_text.setStyle(sf::Text::Italic);
	artist_text.setCharacterSize(24);
	artist_text.setFillColor({255, 255, 255, 150});

	metadata.use_metadata(media);
	if (media.attached_pic())
		metadata.set_album_cover(*media.attached_pic(), {150, 150});
	metadata.set_position({30, 30});

	emplace_layer<audioviz::Layer>("metadata").add_draw({metadata});
}

void ttviz::update(std::span<const float> audio_buffer)
{
	capture_time("fft", sa.execute_fft(fa, audio_buffer));
	capture_time("spectrum_update", ss.update(fa, sa, bp, ip));
	capture_time("ps_update", ps.update(sa));

	if (media.has_video_stream())
	{
		const int frames_to_wait = framerate / media.video_framerate();
		if (vfcount < frames_to_wait)
			++vfcount;
		else
		{
			if (media.read_video_frame(bg_txr))
				bg_spr = bg_txr;
			vfcount = 1;
		}
	}

	color.increment_wheel_time();
}

void ttviz::add_default_effects()
{
	if (const auto bg = get_layer_as<audioviz::PostProcessLayer>("bg"))
	{
		bg->add_effect(
			media.has_video_stream() ? new audioviz::fx::Blur{2.5, 2.5, 5} : new audioviz::fx::Blur{7.5, 7.5, 15});

		if (!media.has_video_stream())
			bg->add_effect(new audioviz::fx::Mult{0.75});

		if (media.attached_pic())
			set_background(*media.attached_pic());
	}

	get_layer_as<audioviz::PostProcessLayer>("particles")->add_effect(new audioviz::fx::Blur{1, 1, 10});
	get_layer_as<audioviz::PostProcessLayer>("spectrum")->add_effect(new audioviz::fx::Blur{3, 3, 10});
}

void ttviz::set_album_cover(const std::string &image_path, const sf::Vector2f size)
{
	metadata.set_album_cover(sf::Texture{image_path}, size);
}

void ttviz::set_background(const sf::Texture &txr)
{
	bg_spr = bg_txr = txr;
	bg_spr.capture_centered_square_view();
	bg_spr.fill_screen(size);
}

void ttviz::set_spectrum_blendmode(const sf::BlendMode &bm)
{
	spectrum_bm = bm;
}

void ttviz::set_fft_size(int fft_size)
{
	fa.set_fft_size(fft_size);
	sa.set_fft_size(fft_size);
}
