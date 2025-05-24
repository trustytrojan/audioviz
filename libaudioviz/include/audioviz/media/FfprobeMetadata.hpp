#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class FfprobeMetadata
{
	json metadata;

public:
	FfprobeMetadata(const std::string &media_url)
	{
		const auto command{"ffprobe -v warning -show_format -show_streams -print_format json '" + media_url + '\''};
		const auto pipe{popen(command.c_str(), "r")};
		if (!pipe)
			throw std::runtime_error{std::string{"popen: "} + strerror(errno)};

		std::ostringstream oss;
		char buffer[128];
		while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
			oss << buffer;

		if (const auto status{pclose(pipe)}; status == -1)
			throw std::runtime_error{std::string{"pclose: "} + strerror(errno)};

		this->metadata = json::parse(oss.str());

		// Assert that required fields exist
		assert(metadata.contains("streams") && metadata["streams"].is_array());
		assert(metadata.contains("format") && metadata["format"].is_object());
	}

	bool hasAudioStream() const
	{
		for (const auto &stream : metadata["streams"])
			if (stream["codec_type"] == "audio")
				return true;
		return false;
	}

	bool hasVideoStream() const
	{
		for (const auto &stream : metadata["streams"])
			if (stream["codec_type"] == "video" && stream["disposition"]["attached_pic"] != 1)
				return true;
		return false;
	}

	int getAudioSampleRate() const
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

	int getVideoFramerate() const
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

	int getAudioChannels() const
	{
		for (const auto &stream : metadata["streams"])
			if (stream["codec_type"] == "audio")
				return stream["channels"];
		return 0;
	}

	bool hasAttachedPic() const
	{
		for (const auto &stream : metadata["streams"])
			if (stream["disposition"]["attached_pic"] == 1)
				return true;
		return false;
	}

	std::string getTitle() const { return metadata["format"]["tags"]["title"]; }
	std::string getArtist() const { return metadata["format"]["tags"]["artist"]; }
};
