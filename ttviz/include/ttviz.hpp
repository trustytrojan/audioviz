#pragma once

#include "audioviz/StereoSpectrum.hpp"
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

#include <audioviz/Base.hpp>
#include <audioviz/ParticleSystem.hpp>
#include <audioviz/SongMetadataDrawable.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/BinPacker.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fft/Interpolator.hpp>
#include <audioviz/fft/StereoAnalyzer.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>

class ttviz : public audioviz::Base
{
	using ParticleShapeType = sf::CircleShape;

	// Media reference
	audioviz::Media &media;
	int sample_rate_hz;
	
	// FFT processors
	audioviz::FrequencyAnalyzer fa;
	audioviz::StereoAnalyzer sa;
	audioviz::BinPacker bp;
	audioviz::Interpolator ip;

	// Color settings
	audioviz::ColorSettings color;

	// Spectrum visualization
	// audioviz::SpectrumDrawable spectrum_left, spectrum_right;
	audioviz::StereoSpectrum ss;
	std::optional<sf::BlendMode> spectrum_bm;

	// Particle system
	audioviz::ParticleSystem<ParticleShapeType> ps;
	int frame_count{};

	// Metadata
	sf::Text title_text{font}, artist_text{font};
	audioviz::SongMetadataDrawable metadata{title_text, artist_text};

	// Background
	sf::Texture video_bg;
	int vfcount{1};

public:
	ttviz(sf::Vector2u size, audioviz::Media &media, int fft_size = 3000, int antialiasing = 4);

	// Configuration methods
	void add_default_effects();
	void set_background(const sf::Texture &texture);
	void set_spectrum_blendmode(const sf::BlendMode &bm);
	void set_album_cover(const std::string &image_path, sf::Vector2f size = {150, 150});
	void set_fft_size(int fft_size);
	
	// Getters for configuration
	inline audioviz::FrequencyAnalyzer &get_fa() { return fa; }
	inline audioviz::BinPacker &get_bp() { return bp; }
	inline audioviz::Interpolator &get_ip() { return ip; }
	inline audioviz::ColorSettings &get_color() { return color; }
	inline audioviz::ParticleSystem<ParticleShapeType> &get_ps() { return ps; }
	inline audioviz::StereoSpectrum &get_ss() { return ss; }

private:
	void update(std::span<const float> audio_buffer) override;
};
