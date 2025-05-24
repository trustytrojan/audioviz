#pragma once

#include <SFML/Graphics.hpp>
#include <av/MediaReader.hpp>

namespace audioviz::media
{

class Media
{
public:
	static Media *create(const std::string &url, sf::Vector2u video_size = {});

private:
	std::vector<float> _audio_buffer;

public:
	const std::string url;
	const sf::Vector2u video_size;

	Media(const std::string &url, sf::Vector2u video_size = {});
	virtual ~Media() = default; // fixes clangd warning

	/**
	 * Read `samples` audio SAMPLES (not FRAMES) from the underlying source
	 * into `buf`. Typically not used publically.
	 */
	virtual size_t read_audio_samples(float *buf, int samples) = 0;

	/**
	 * Read a video frame of size `video_size` (passed in the constructor)
	 * from the underlying source into `txr`.
	 */
	virtual bool read_video_frame(sf::Texture &txr) = 0;

	virtual int audio_sample_rate() const = 0;
	virtual int audio_channels() const = 0;
	virtual bool has_video_stream() const = 0;
	virtual int video_framerate() const = 0;
	virtual const std::optional<sf::Texture> &attached_pic() const = 0;
	virtual std::string title() const = 0;
	virtual std::string artist() const = 0;

	/**
	 * Guarantees that up to `frames` audio frames are buffered and accessible
	 * via `audio_buffer()`.
	 */
	void buffer_audio(const int frames);

	/**
	 * Access the audio buffer.
	 */
	inline const std::vector<float> &audio_buffer() const { return _audio_buffer; }

	/**
	 * Erase `frames` audio frames from the buffer.
	 */
	void audio_buffer_erase(const int frames);

	/**
	 * Returns the number of audio frames (dependent on `audio_channels()`) in the buffer.
	 */
	inline int audio_buffer_frames() const { return _audio_buffer.size() / audio_channels(); }
};

} // namespace audioviz::media
