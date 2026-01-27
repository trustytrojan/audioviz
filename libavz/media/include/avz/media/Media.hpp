#pragma once

#include <optional>
#include <span>
#include <string>
#include <vector>

namespace avz
{

/**
 * Generic media file interface with access to audio, video, and metadata.
 * Provides a manual audio buffer to guarantee a certain amount of audio
 * regardless of the implementation's reading quirks.
 */
class Media
{
	std::vector<float> _audio_buffer;

public:
	const std::string url;
	Media(const std::string &url);
	virtual ~Media() = default;

	/**
	 * Read `samples` audio SAMPLES (NOT FRAMES) from the underlying source
	 * into `buf`. Typically not used publicly. You should prefer using the
	 * built-in audio buffer instead.
	 */
	virtual size_t read_audio_samples(float *buf, int samples) = 0;

	/**
	 * Read a video frame of size `video_size` (passed in the constructor)
	 * from the underlying source into `txr`.
	 */
	virtual bool read_video_frame(std::vector<std::byte> &txr) = 0;

	virtual int audio_sample_rate() const = 0;
	virtual int audio_channels() const = 0;
	virtual bool has_video_stream() const = 0;
	virtual int video_framerate() const = 0;
	virtual const std::optional<std::vector<std::byte>> &attached_pic() const = 0;
	virtual std::string title() const = 0;
	virtual std::string artist() const = 0;

	/**
	 * Erase `frames` audio frames from the buffer.
	 * This helps "slide" the audio window forward by `frames`, ensuring
	 * consistent playback when rendering at a specific framerate.
	 */
	void consume_audio(const int frames);

	/**
	 * Attempts to buffer `frames` audio frames from the underlying source.
	 * On success, returns a span pointing to the buffered audio.
	 * Otherwise returns an empty optional.
	 */
	std::optional<std::span<const float>> read_audio(int frames);
};

} // namespace avz
