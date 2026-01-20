#include <avz/media/FfprobeMetadata.hpp>
#include <avz/util.hpp>
#include <iostream>
#include <sstream>

FfprobeMetadata::FfprobeMetadata(const std::string &media_url)
{
	const auto command{"ffprobe -v warning -show_format -show_streams -print_format json \"" + media_url + '"'};
	const auto ffprobe{audioviz::util::popen_utf8(command, POPEN_R_MODE)};
	if (!ffprobe)
		throw std::runtime_error{std::string{"popen: "} + strerror(errno)};

	std::ostringstream oss;
	char buffer[128];
	while (fgets(buffer, sizeof(buffer), ffprobe) != nullptr)
		oss << buffer;

	switch (const auto status{audioviz::util::pclose_utf8(ffprobe)})
	{
	case -1:
		throw std::runtime_error{std::string{"pclose: "} + strerror(errno)};
	case 0:
		break;
	default:
		std::cerr << __func__ << ": pclose returned: " << status << '\n';
		return;
	}

	metadata = json::parse(oss.str());

	// Assert that required fields exist
	assert(metadata.contains("streams") && metadata["streams"].is_array());
	assert(metadata.contains("format") && metadata["format"].is_object());
}

bool FfprobeMetadata::hasAudioStream() const
{
	for (const auto &stream : metadata["streams"])
		if (stream["codec_type"] == "audio")
			return true;
	return false;
}

bool FfprobeMetadata::hasVideoStream() const
{
	for (const auto &stream : metadata["streams"])
		if (stream["codec_type"] == "video" && stream["disposition"]["attached_pic"] != 1)
			return true;
	return false;
}

int FfprobeMetadata::getAudioSampleRate() const
{
	for (const auto &stream : metadata["streams"])
	{
		if (stream["codec_type"] == "audio")
		{
			std::string sr_str = stream["sample_rate"];
			return std::stoi(sr_str);
		}
	}
	return 0;
}

int FfprobeMetadata::getVideoFramerate() const
{
	for (const auto &stream : metadata["streams"])
	{
		if (stream["codec_type"] == "video")
		{
			std::string fr_str = stream["avg_frame_rate"];
			auto pos = fr_str.find('/');
			if (pos != std::string::npos)
			{
				int num = std::stoi(fr_str.substr(0, pos));
				int denom = std::stoi(fr_str.substr(pos + 1));
				if (denom != 0)
					return num / denom;
			}
		}
	}
	return 0;
}

int FfprobeMetadata::getAudioChannels() const
{
	for (const auto &stream : metadata["streams"])
		if (stream["codec_type"] == "audio")
			return stream["channels"];
	return 0;
}

bool FfprobeMetadata::hasAttachedPic() const
{
	for (const auto &stream : metadata["streams"])
		if (stream["disposition"]["attached_pic"] == 1)
			return true;
	return false;
}

std::string FfprobeMetadata::getTitle() const
{
	const auto &tags = metadata["format"]["tags"];
	if (tags.contains("title"))
		return tags["title"];
	if (tags.contains("TITLE"))
		return tags["TITLE"];
	return {};
}

std::string FfprobeMetadata::getArtist() const
{
	const auto &tags = metadata["format"]["tags"];
	if (tags.contains("artist"))
		return tags["artist"];
	if (tags.contains("ARTIST"))
		return tags["ARTIST"];
	return {};
}
