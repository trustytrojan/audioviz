#pragma once

#include <avz/media/FfprobeMetadata.hpp>
#include <avz/media/Media.hpp>

namespace avz
{

/**
 * Implementation of `Media` using the `ffmpeg` binary for audio/video, and
 * `ffprobe` for metadata. Uses hardware-acceleration when possible for video
 * decoding and scaling.
 */
class FfmpegPopenMedia : public Media
{
public:
	static std::optional<std::vector<std::byte>> getAttachedPicture(const std::string &mediaPath);

private:
	const unsigned scaled_width{}, scaled_height{};
	FILE *audio{}, *video{};
	FfprobeMetadata metadata;
	std::optional<std::vector<std::byte>> _attached_pic;

	void init_audio(float start_time_sec = {});
	void init_video();

public:
	/**
	 * Open the media at the provided URL. Optionally provide the desired video size
	 * for video frames to be scaled to by `ffmpeg`.
	 */
	FfmpegPopenMedia(const std::string &url, unsigned scaled_width, unsigned scaled_height, float start_time_sec = {});
	FfmpegPopenMedia(const std::string &url, float start_time_sec = {});
	~FfmpegPopenMedia();

	size_t read_audio_samples(float *buf, int samples) override;
	bool read_video_frame(std::vector<std::byte> &buf) override;

	inline int audio_sample_rate() const override { return metadata.getAudioSampleRate(); }
	inline int audio_channels() const override { return metadata.getAudioChannels(); }
	inline bool has_video_stream() const override { return metadata.hasVideoStream(); }
	inline int video_framerate() const override { return metadata.getVideoFramerate(); }
	inline std::string title() const override { return metadata.getTitle(); }
	inline std::string artist() const override { return metadata.getArtist(); }
	inline const std::optional<std::vector<std::byte>> &attached_pic() const override { return _attached_pic; }
};

} // namespace avz
