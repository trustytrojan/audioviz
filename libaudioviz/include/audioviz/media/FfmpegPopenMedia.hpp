#pragma once

#include "FfprobeMetadata.hpp"
#include "Media.hpp"

namespace audioviz
{

class FfmpegPopenMedia : public Media
{
	FILE *audio{}, *video{};
	FfprobeMetadata metadata;
	std::optional<sf::Texture> _attached_pic;

public:
	FfmpegPopenMedia(const std::string &url, sf::Vector2u video_size = {});
	~FfmpegPopenMedia();

	size_t read_audio_samples(float *buf, int samples) override;
	bool read_video_frame(sf::Texture &txr) override;

	inline int audio_sample_rate() const override { return metadata.getAudioSampleRate(); }
	inline int audio_channels() const override { return metadata.getAudioChannels(); }
	inline bool has_video_stream() const override { return metadata.hasVideoStream(); }
	inline int video_framerate() const override { return metadata.getVideoFramerate(); }
	inline std::string title() const override { return metadata.getTitle(); }
	inline std::string artist() const override { return metadata.getArtist(); }
	inline const std::optional<sf::Texture> &attached_pic() const override { return _attached_pic; }
};

} // namespace audioviz::media
