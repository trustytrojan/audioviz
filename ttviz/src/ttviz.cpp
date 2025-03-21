#include <iostream>

#include <audioviz/fx/Blur.hpp>
#include <audioviz/fx/Mult.hpp>
#include <audioviz/media/FfmpegCliBoostMedia.hpp>
#include "ttviz.hpp"

#define capture_time(label, code)            \
	{                                        \
		sf::Clock _clock;                    \
		code;                                \
		capture_elapsed_time(label, _clock); \
	}

ttviz::ttviz(
	const sf::Vector2u size, const std::string &media_url, FA &fa, CS &color, SS &ss, PS &ps, const int antialiasing)
	: Base{size, new audioviz::media::FfmpegCliBoostMedia{media_url, size}},
	  fa{fa},
	  color{color},
	  ss{ss},
	  scope{{{}, (sf::Vector2i)size}, color},
	  ps{ps},
	  video_bg{size}
{
	// for now only stereo is supported
	// this will be moved into its own class eventually
	if (media->astream().nb_channels() != 2)
		throw std::runtime_error("only stereo audio is supported!");

	// default spectrum margin
	// this sets the StereoSpectrum's rectangle (necessary for it to render)
	set_spectrum_margin(10);

	// create stereo "mirror" effect
	ss.set_left_backwards(true);

	metadata_init();
	layers_init(antialiasing);

	// default metadata position
	// needs to be called AFTER album cover is set because the text position depends on album cover size
	metadata.set_position({30, 30});

	// scope setup
	scope.set_shape_spacing(0);
	scope.set_shape_width(2);
	scope.set_fill_in(true);

	if (media->attached_pic())
	{
		metadata.set_album_cover(*media->attached_pic(), {150, 150});
		set_background(*media->attached_pic()); // this is safe bc we already called `layers_init`
	}

	// VERY IMPORTANT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// now that two things are dependent on different amounts of audio, take the max of their dependencies
	set_audio_frames_needed(std::max(fa.get_fft_size(), (int)scope.get_shape_count()));
}

void ttviz::perform_fft()
{
	ss.configure_analyzer(sa);
	audioviz::Base::perform_fft(fa, sa);
}

void ttviz::layers_init(const int antialiasing)
{
	{ // bg layer
		auto &bg = add_layer("bg", antialiasing);

		if (media->vstream()) // set_orig_cb() to draw video frames on the layer
		{
			// round the framerate bc sometimes it's 29.97
			const int video_framerate = std::round(av_q2d(media->vstream()->get()->avg_frame_rate));
			bg.set_orig_cb(
				[this, frames_to_wait{get_framerate() / video_framerate}](auto &orig_rt)
				{
					if (vfcount < frames_to_wait)
						++vfcount;
					else
					{
						if (media->read_video_frame(video_bg))
							orig_rt.draw(sf::Sprite{video_bg});
						else
							std::cout << "media->read_video_frame returned false????????\n";
						vfcount = 1; // ALWAYS RESET TO 1 OTHERWISE THE IF CHECK ABOVE DOESN'T MAKE SENSE
					}
					// orig_rt.display();
				});
		}

		else
			// we only have one image; don't run effects in Layer::full_lifecycle
			// which means we need to prepopulate the `fx_rt` of this layer with something...
			// set_background() handles this
			bg.set_auto_fx(false);

		bg.set_fx_cb(audioviz::Layer::DRAW_FX_RT);
	}

	/*{ // scope layer
		auto &scope_layer = add_layer("scope", antialiasing);
		scope_layer.set_orig_cb(
			[&](auto &orig_rt)
			{
				float left_channel[scope.get_shape_count()];
				for (int i = 0; i < scope.get_shape_count(); ++i)
					left_channel[i] = media->audio_buffer()[i * media->astream().nb_channels() + 0];
				scope.update({left_channel, scope.get_shape_count()});
				orig_rt.clear(sf::Color::Transparent);
				orig_rt.draw(scope);
				orig_rt.display();
			});
		scope_layer.set_fx_cb(viz::Layer::DRAW_FX_RT);
	}*/

	{ // particles layer
		auto &particles = add_layer("particles", antialiasing);
		particles.drawables.emplace_back(&ps);
		particles.set_orig_cb(
			[&](auto &orig_rt)
			{
				// this HAS to be done before particles or spectrum, as both depend
				// on fft being performed on the current audio buffer for this frame
				perform_fft();

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
	}

	{ // spectrum layer
		auto &spectrum = add_layer("spectrum", antialiasing);
		spectrum.drawables.emplace_back(&ss);
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
					target.draw(fx_rt.sprite(), sf::BlendAdd);
					if constexpr (std::is_same_v<BarType, sf::CircleShape>)
						// redraw the entire ss because antialiased edges have dark pixels with 1 alpha...
						target.draw(ss);
					else
					 	target.draw(orig_rt.sprite());
				}
			});
	}
}

void ttviz::add_default_effects()
{
	if (const auto bg = get_layer("bg"))
	{
		// clang-format off
		bg->effects.emplace_back(
			media->vstream()
				? new audioviz::fx::Blur{2.5, 2.5, 5}
				: new audioviz::fx::Blur{7.5, 7.5, 15});
		// clang-format on
		if (!media->vstream())
			bg->effects.emplace_back(new audioviz::fx::Mult{0.75});
		if (media->attached_pic())
			// this will set the background WITH the blur affect we just added
			set_background(*media->attached_pic());
	}

	if (const auto particles = get_layer("particles"))
		particles->effects.emplace_back(new audioviz::fx::Blur{1, 1, 10});

	if (const auto spectrum = get_layer("spectrum"))
		spectrum->effects.emplace_back(new audioviz::fx::Blur{1, 1, 20});
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
	metadata.use_metadata(*media);
}

void ttviz::set_spectrum_margin(const int margin)
{
	ss.set_rect({{margin, margin}, {size.x - 2 * margin, size.y - 2 * margin}});
	sa.resize(ss.get_bar_count());
}

void ttviz::set_background(const sf::Texture &txr)
{
	const auto bg = get_layer("bg");
	if (!bg)
		throw std::runtime_error("no background layer present!");
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

bool ttviz::next_frame()
{
	// pasting this here for imgui!!!!!!!!!!!!!!!!!!!!!
	set_audio_frames_needed(std::max(fa.get_fft_size(), (int)scope.get_shape_count()));
	const auto next_frame_ready = Base::next_frame();
	color.increment_wheel_time();
	return next_frame_ready;
}

void ttviz::draw(sf::RenderTarget &target, const sf::RenderStates states) const
{
	Base::draw(target, states);
	target.draw(metadata, states);
}
