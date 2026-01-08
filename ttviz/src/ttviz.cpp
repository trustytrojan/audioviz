#include "ttviz.hpp"
#include <audioviz/fx/Blur.hpp>
#include <audioviz/fx/Mult.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <iostream>

#define capture_time(label, code)            \
	if (timing_text_enabled())               \
	{                                        \
		sf::Clock _clock;                    \
		code;                                \
		capture_elapsed_time(label, _clock); \
	}                                        \
	else                                     \
		code;

ttviz::ttviz(const sf::Vector2u size, audioviz::Media &media, FA &fa, CS &color, SS &ss, PS &ps, const int antialiasing)
	: Base{size},
	  media{media},
	  fa{fa},
	  color{color},
	  ss{ss},
	  ps{ps},
	  video_bg{size}
{
	// create stereo "mirror" effect
	ss.set_left_backwards(true);
	ss.set_rect({{}, {size.x, size.y - 10}});

	metadata_init();
	layers_init(antialiasing);

	// VERY IMPORTANT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// now that two things are dependent on different amounts of audio, take the max of their dependencies
	set_audio_frames_needed(std::max(fa.get_fft_size(), afpvf));
}

void ttviz::update(std::span<const float> audio_buffer)
{
	// imgui calls go here
#ifdef AUDIOVIZ_IMGUI
	ps.draw_imgui();
#endif

	ss.configure_analyzer(sa);
	capture_time("fft", sa.analyze(fa, audio_buffer.data(), true));
	color.increment_wheel_time();
}

void ttviz::layers_init(const int antialiasing)
{
	// bg layer
	if (media.attached_pic() || media.has_video_stream())
	{
		auto &bg = add_layer("bg", antialiasing);

		if (media.has_video_stream()) // set_orig_cb() to draw video frames on the layer
		{
			bg.set_orig_cb(
				[this, frames_to_wait{get_framerate() / media.video_framerate()}](auto &orig_rt)
				{
					if (vfcount < frames_to_wait)
						++vfcount;
					else
					{
						if (media.read_video_frame(video_bg))
							orig_rt.draw(sf::Sprite{video_bg});
						else
							std::cerr << "media->read_video_frame returned false????????\n";
						vfcount = 1; // ALWAYS RESET TO 1 OTHERWISE THE IF CHECK ABOVE DOESN'T MAKE SENSE
					}
					orig_rt.display();
				});
		}
		else if (media.attached_pic())
		{
			// we only have one image; don't run effects in Layer::full_lifecycle
			// which means we need to prepopulate the `fx_rt` of this layer with something...
			// set_background() handles this
			bg.set_auto_fx(false);
			set_background(*media.attached_pic());
		}
	}

	auto &particles = add_layer("particles", antialiasing);
	particles.add_drawable(&ps);
	particles.set_orig_cb(
		[&](auto &orig_rt)
		{
			// lock the tickrate of the particles at 60hz for non-60fps output
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
		[&](auto &orig_rt, auto &fx_rt, auto &target)
		{
			target.draw(fx_rt.sprite(), sf::BlendAdd);
			target.draw(orig_rt.sprite(), sf::BlendAdd);
		});

	auto &spectrum = add_layer("spectrum", antialiasing);
	spectrum.add_drawable(&ss);
	spectrum.set_orig_cb([&](auto &) { ss.update(sa); });
	spectrum.set_fx_cb(
		[&](auto &orig_rt, auto &fx_rt, auto &target)
		{
			if (spectrum_bm)
			{
				target.draw(fx_rt.sprite(), *spectrum_bm);
				target.draw(orig_rt.sprite(), *spectrum_bm);
			}
			else
			{
				target.draw(fx_rt.sprite(), audioviz::util::GreatAmazingBlendMode);
				if constexpr (std::is_same_v<BarType, sf::CircleShape>)
					// redraw the entire ss because antialiased edges have dark pixels with 1 alpha...
					target.draw(ss);
				else
					target.draw(orig_rt.sprite());
			}
		});
}

void ttviz::add_default_effects()
{
	if (const auto bg = get_layer("bg"))
	{
		// clang-format off
		bg->add_effect(
			media.has_video_stream()
				? new audioviz::fx::Blur{2.5, 2.5, 5}
				: new audioviz::fx::Blur{7.5, 7.5, 15});
		// clang-format on
		if (!media.has_video_stream())
			bg->add_effect(new audioviz::fx::Mult{0.75});
		if (media.attached_pic())
			// this will set the background WITH the blur affect we just added
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

void ttviz::metadata_init()
{
	// set style, fontsize, and color
	title_text.setStyle(sf::Text::Bold | sf::Text::Italic);
	title_text.setCharacterSize(24);
	title_text.setFillColor({255, 255, 255, 150});

	artist_text.setStyle(sf::Text::Italic);
	artist_text.setCharacterSize(24);
	artist_text.setFillColor({255, 255, 255, 150});

	// set text using stream/format metadata
	metadata.use_metadata(media);

	if (media.attached_pic())
		metadata.set_album_cover(*media.attached_pic(), {150, 150});

	// default metadata position
	// needs to be called AFTER album cover is set because the text position depends on album cover size
	metadata.set_position({30, 30});

	// metadata should be drawn on top of the final canvas for blending purposes
	add_final_drawable(metadata);
}

void ttviz::set_background(const sf::Texture &txr)
{
	const auto bg = get_layer("bg");
	if (!bg)
		throw std::runtime_error{"no background layer present!"};
	audioviz::Sprite spr{txr};

	// i do this because of *widescreen* youtube thumbnails containing *square* album covers
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
