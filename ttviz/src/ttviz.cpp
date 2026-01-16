#include "ttviz.hpp"
#include <audioviz/fx/Blur.hpp>
#include <audioviz/fx/Mult.hpp>
#include <audioviz/util.hpp>
#include <stdexcept>

ttviz::ttviz(const sf::Vector2u size, audioviz::Media &media, const int fft_size, const int antialiasing)
	: Base{size},
	  media{media},
	  sample_rate_hz{media.audio_sample_rate()},
	  fa{fft_size},
	  sa{sample_rate_hz, fft_size},
	  ss{{{}, {size.x, size.y - 10}}, color},
	  ps{{{}, (sf::Vector2i)size}, 50, get_framerate()}
{
	// Check for stereo audio
	if (media.audio_channels() != 2)
		throw std::runtime_error{
			"ttviz requires stereo (2-channel) audio; got " + std::to_string(media.audio_channels()) + " channel(s)"};

	set_audio_frames_needed(fft_size);

	// Configure spectrum
	ss.set_left_backwards(true);

	// Setup metadata
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
	add_final_drawable(metadata);

	// Setup layers
	// Background layer (for album art or video)
	if (media.attached_pic() || media.has_video_stream())
	{
		auto &bg = add_layer("bg", antialiasing);

		if (media.has_video_stream())
		{
			video_bg = sf::Texture{size};
			bg.set_orig_cb(
				[this, &media](auto &orig_rt)
				{
					const int frames_to_wait = get_framerate() / media.video_framerate();
					if (vfcount < frames_to_wait)
						++vfcount;
					else
					{
						if (media.read_video_frame(video_bg))
							orig_rt.draw(sf::Sprite{video_bg});
						vfcount = 1;
					}
					orig_rt.display();
				});
		}
		else if (media.attached_pic())
		{
			bg.set_auto_fx(false);
			set_background(*media.attached_pic());
		}
	}

	// Particles layer
	auto &particles = add_layer("particles", antialiasing);
	particles.add_draw({ps});
	particles.set_orig_cb(
		[this](auto &)
		{
			const auto framerate = get_framerate();

			if (framerate < 60)
				ps.update(sa, {.multiplier = 60.f / framerate});
			else if (framerate == 60)
				ps.update(sa);
			else if (framerate > 60 && frame_count >= (framerate / 60.))
			{
				ps.update(sa);
				frame_count = 0;
			}
			++frame_count;
		});
	particles.set_fx_cb(
		[](auto &orig_rt, auto &fx_rt, auto &target)
		{
			target.draw(fx_rt.sprite(), sf::BlendAdd);
			target.draw(orig_rt.sprite(), sf::BlendAdd);
		});

	// Spectrum layer
	auto &spectrum = add_layer("spectrum", antialiasing);
	spectrum.add_draw({ss});
	spectrum.set_fx_cb(
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
}

void ttviz::update(std::span<const float> audio_buffer)
{
	// Execute FFT for both channels
	capture_time("fft", sa.execute_fft(fa, audio_buffer));

	// Update spectrum for both channels
	capture_time("spectrum_update", ss.update(fa, sa, bp, ip));

	// Update color wheel
	color.increment_wheel_time();
}

void ttviz::add_default_effects()
{
	if (const auto bg = get_layer("bg"))
	{
		bg->add_effect(
			media.has_video_stream() ? new audioviz::fx::Blur{2.5, 2.5, 5} : new audioviz::fx::Blur{7.5, 7.5, 15});

		if (!media.has_video_stream())
			bg->add_effect(new audioviz::fx::Mult{0.75});

		if (media.attached_pic())
			set_background(*media.attached_pic());
	}

	if (const auto particles = get_layer("particles"))
		particles->add_effect(new audioviz::fx::Blur{1, 1, 10});

	if (const auto spectrum = get_layer("spectrum"))
		spectrum->add_effect(new audioviz::fx::Blur{3, 3, 10});
}

void ttviz::set_album_cover(const std::string &image_path, const sf::Vector2f size)
{
	metadata.set_album_cover(sf::Texture{image_path}, size);
}

void ttviz::set_background(const sf::Texture &txr)
{
	const auto bg = get_layer("bg");
	if (!bg)
		throw std::runtime_error{"no background layer present!"};

	audioviz::Sprite spr{txr};
	spr.capture_centered_square_view();
	spr.fill_screen(size);

	bg->orig_draw(spr);
	bg->orig_display();
	bg->apply_fx();
}

void ttviz::set_spectrum_blendmode(const sf::BlendMode &bm)
{
	spectrum_bm = bm;
}

void ttviz::set_fft_size(int fft_size)
{
	fa.set_fft_size(fft_size);
	sa.set_fft_size(fft_size);
	set_audio_frames_needed(fft_size);
}
