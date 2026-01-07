#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

#include <audioviz/Base.hpp>
#include <audioviz/ParticleSystem.hpp>
#include <audioviz/ScopeDrawable.hpp>
#include <audioviz/SongMetadataDrawable.hpp>
#include <audioviz/StereoSpectrum.hpp>
#include <audioviz/VerticalBar.hpp>

class ttviz : public audioviz::Base
{
	using BarType = audioviz::VerticalBar;
	using ParticleShapeType = sf::CircleShape;
	using FA = audioviz::FrequencyAnalyzer;
	using SA = audioviz::StereoAnalyzer;
	using CS = audioviz::ColorSettings;
	using SS = audioviz::StereoSpectrum<BarType>;
	using SD = audioviz::ScopeDrawable<sf::RectangleShape>;
	using PS = audioviz::ParticleSystem<ParticleShapeType>;

	// used for updating the particle system at 60Hz rate when framerate > 60
	int frame_count{}, vfcount{1};

	// Base and Media are now decoupled, we need it for ttviz though
	audioviz::Media &media;

	// fft processor
	FA &fa;
	SA sa;

	// color settings
	CS &color;

	// stereo spectrum
	SS &ss;
	std::optional<sf::BlendMode> spectrum_bm;

	// particle system
	PS &ps;

	// metadata-related fields (font from base_audioviz)
	sf::Text title_text{font}, artist_text{font};
	audioviz::SongMetadataDrawable metadata{title_text, artist_text};

	sf::Texture video_bg;

public:
	/**
	 * @param size size of the output; recommended to match your `sf::RenderTarget`'s size
	 * @param media media source
	 * @param fa reference to your own `FrequencyAnalyzer`
	 * @param cs reference to your own `audioviz::ColorSettings`
	 * @param ss reference to your own `audioviz::StereoSpectrum<BarType>`
	 * @param ps reference to your own `audioviz::ParticleSystem<ParticleShapeType>`
	 * @param antialiasing antialiasing level to use for round shapes
	 */
	ttviz(sf::Vector2u size, audioviz::Media &media, FA &fa, CS &color, SS &ss, PS &ps, int antialiasing = 4);

	/**
	 * Add default effects to the `bg`, `spectrum`, and `particles` layers, if they exist.
	 */
	void add_default_effects();

	// set background image with optional effects: blur and color-multiply
	void set_background(const sf::Texture &texture);

	// set blend mode for spectrum against target
	void set_spectrum_blendmode(const sf::BlendMode &);

	void set_album_cover(const std::string &image_path, sf::Vector2f size = {150, 150});

	/**
	 * Set the number of audio samples used for frequency analysis.
	 * @note Calls `set_fft_size` on the `FrequencyAnalyzer` you passed in the constructor.
	 */
	void set_fft_size(int fft_size);

private:
	void metadata_init();
	void layers_init(int);
	virtual void update(std::span<const float> /*audio_buffer*/) override;
};
