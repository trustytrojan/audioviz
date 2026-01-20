#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// not sure if this should even be namespaced/part of the audioviz c++ api
class FfprobeMetadata
{
	json metadata;

public:
	FfprobeMetadata(const std::string &media_url);
	bool hasAudioStream() const;
	bool hasVideoStream() const;
	int getAudioSampleRate() const;
	int getVideoFramerate() const;
	int getAudioChannels() const;
	bool hasAttachedPic() const;
	std::string getTitle() const;
	std::string getArtist() const;
};
