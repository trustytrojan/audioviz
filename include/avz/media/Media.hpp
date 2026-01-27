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
	 * Read audio SAMPLES (NOT frames) from the underlying source into `buf`.
	 * Typically not used publicly, as  You should prefer using `read_audio` instead.
	 */
	virtual size_t read_audio_samples(float *buf, int samples) = 0;

	/**
	 * Read a video frame from the underlying source into `buf`.
	 * The buffer will be resized to match the video frame's size.
	 * Returns whether the read was successful.
	 */
	virtual bool read_video_frame(std::vector<std::byte> &buf) = 0;

	virtual int audio_sample_rate() const = 0;
	virtual int audio_channels() const = 0;
	virtual bool has_video_stream() const = 0;
	virtual int video_framerate() const = 0;
	virtual const std::optional<std::vector<std::byte>> &attached_pic() const = 0;
	virtual std::string title() const = 0;
	virtual std::string artist() const = 0;

	/**
	 * Erase the first `frames` audio frames from the buffer. This is
	 * used in tandem with `read_audio` to "move" the audio buffer
	 * forward by the `frames` you have already used.
	 */
	void consume_audio(const int frames);

	/**
	 * Attempts to buffer `frames` audio frames from the underlying source.
	 * On success, returns a span pointing to the audio buffer sized by
	 * the amount of `frames` requested.
	 * Otherwise returns an empty optional.
	 *
	 * NOTE: You must call `consume_audio` to erase the audio you no longer
	 * need from the buffer, otherwise this method will keep returning the
	 * same audio without reading new data from the implementation.
	 */
	std::optional<std::span<const float>> read_audio(int frames);
};

} // namespace avz
