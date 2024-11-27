#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

#ifdef AUDIOVIZ_PORTAUDIO
#include <portaudio.hpp>
#endif

#include "base_audioviz.hpp"

// #include "viz/Layer.hpp"
#include "viz/ParticleSystem.hpp"
#include "viz/ScopeDrawable.hpp"
#include "viz/SongMetadataDrawable.hpp"
#include "viz/StereoSpectrum.hpp"
#include "viz/VerticalBar.hpp"

// #include "media/Media.hpp"

namespace viz
{
class Layer;
}

class audioviz : public base_audioviz
{
public:
	// audioviz output size. cannot be changed, so make sure your window is not resizable.
	// const sf::Vector2u size;

private:
	using BarType = viz::VerticalBar;
	using ParticleShapeType = sf::CircleShape;

	using FA = tt::FrequencyAnalyzer;
	using CS = viz::ColorSettings;
	using SS = viz::StereoSpectrum<BarType>;
	using SD = viz::ScopeDrawable<sf::RectangleShape>;
	using PS = viz::ParticleSystem<ParticleShapeType>;

	int fft_size{3000};

	// used for updating the particle system at 60Hz rate when framerate > 60
	int frame_count{}, vfcount{1};

	// fft processor
	FA &fa;
	tt::StereoAnalyzer sa;

	// color settings
	CS &color;

	// stereo spectrum
	SS &ss;
	std::optional<sf::BlendMode> spectrum_bm;

	// scope
	SD scope;

	// particle system
	PS &ps;

	// metadata-related fields (font from base_audioviz)
	sf::Text title_text{font}, artist_text{font};
	viz::SongMetadataDrawable metadata{title_text, artist_text};

	sf::Texture video_bg;

public:
	// need to do this outside of the constructor otherwise the texture is broken?
	void use_attached_pic_as_bg();

	/**
	 * @param size size of the output; recommended to match your `sf::RenderTarget`'s size
	 * @param media_url url to media source. must contain an audio stream
	 * @param fa reference to your own `tt::FrequencyAnalyzer`
	 * @param cs reference to your own `viz::ColorSettings`
	 * @param ss reference to your own `viz::StereoSpectrum<BarType>`
	 * @param ps reference to your own `viz::ParticleSystem<ParticleShapeType>`
	 * @param antialiasing antialiasing level to use for round shapes
	 */
	audioviz(sf::Vector2u size, const std::string &media_url, FA &fa, CS &color, SS &ss, PS &ps, int antialiasing = 4);

	/**
	 * Add default effects to the `bg`, `spectrum`, and `particles` layers, if they exist.
	 */
	void add_default_effects();

	/**
	 * Prepare a frame to be drawn with `draw()`.
	 * This method does all the work to produce a frame.
	 * @return Whether another frame can be prepared
	 */
	bool prepare_frame();

	void draw(sf::RenderTarget &target, sf::RenderStates) const override;

	// set background image with optional effects: blur and color-multiply
	void set_background(const sf::Texture &texture);

	// set margins around the output size for the spectrum to respect
	void set_spectrum_margin(int margin);

	// set blend mode for spectrum against target
	void set_spectrum_blendmode(const sf::BlendMode &);

	void set_album_cover(const std::string &image_path, sf::Vector2f size = {150, 150});

	/**
	 * Set the number of audio samples used for frequency analysis.
	 * @note Calls `set_fft_size` on the `tt::FrequencyAnalyzer` you passed in the constructor.
	 */
	void set_fft_size(int fft_size);

private:
	void metadata_init();
	void layers_init(int);
	void perform_fft();
};
