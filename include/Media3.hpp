#pragma once

#include <SFML/Graphics.hpp>
#include <av/MediaReader.hpp>
#include <iostream>

class Media3
{
	FILE *audio{nullptr}, *video{nullptr};
	av::MediaReader format;
	av::Stream _astream{format.find_best_stream(AVMEDIA_TYPE_AUDIO)};
	std::optional<av::Stream> _vstream;
	std::optional<sf::Texture> _attached_pic;

public:
	Media3(const std::string &url, const bool need_video)
		: format{url}
	{
		try_read_attached_pic();
		try_find_video_stream();
		create_audio_decoder(url);
		if (_vstream && need_video)
			create_video_decoder(url);
	}

	~Media3()
	{
		if (audio && pclose(audio) == -1)
			perror("pclose");
		if (video && pclose(video) == -1)
			perror("pclose");
	}

	const av::Stream &astream() const { return _astream; }
	const std::optional<av::Stream> &vstream() const { return _vstream; }
	const std::optional<sf::Texture> &attached_pic() const { return _attached_pic; }

	size_t read_audio_samples(float *const buf, const int samples) const
	{
		if (!audio)
			throw std::logic_error{"no audio stream"};
		return fread(buf, sizeof(float), samples, audio);
	}

	size_t read_audio_frames(float *const buf, const int frames) const
	{
		return read_audio_samples(buf, frames * _astream.nb_channels()) / _astream.nb_channels();
	}

	void read_video_frame(sf::Texture &txr) const
	{
		if (!video)
			throw std::logic_error{"no video stream available!"};
		const auto bytes_to_read = 4 * txr.getSize().x * txr.getSize().y;
		uint8_t buf[bytes_to_read];
		if (fread(buf, sizeof(uint8_t), bytes_to_read, video) < bytes_to_read)
			return;
		txr.update(buf);
	}

private:
	void try_read_attached_pic()
	{
		const auto &streams = format.streams();
		if (const auto itr = std::ranges::find_if(
				streams, [](const auto &s) { return s->disposition & AV_DISPOSITION_ATTACHED_PIC; });
			itr != streams.cend())
		{
			const auto &stream = *itr;
			_attached_pic = {stream->attached_pic.data, stream->attached_pic.size};
		}
	}

	void try_find_video_stream()
	{
		try
		{
			const auto stream = format.find_best_stream(AVMEDIA_TYPE_VIDEO);
			// we don't want to re-decode the attached pic stream
			if (!(stream->disposition & AV_DISPOSITION_ATTACHED_PIC))
				_vstream = stream;
		}
		catch (const av::Error &e)
		{
			std::cerr << e.what() << '\n';
		}
	}

	void create_audio_decoder(const std::string &url)
	{
		std::ostringstream ss;
		ss << "ffmpeg ";

		// only log warnings
		ss << "-v warning ";

		// input file
		ss << "-hwaccel auto ";
		if (url.contains('\''))
			ss << "-i \"" << url << "\" ";
		else
			ss << "-i '" << url << "' ";

		// audio output
		ss << "-f f32le ";
		ss << "-acodec pcm_f32le ";
		ss << "-";

		if (!(audio = popen(ss.str().c_str(), "r")))
			throw std::runtime_error{std::string{"popen: "} + strerror(errno)};
	}

	void create_video_decoder(const std::string &url)
	{
		std::ostringstream ss;
		ss << "ffmpeg ";

		// only log warnings
		ss << "-v warning ";

		// input file
		ss << "-hwaccel auto ";
		if (url.contains('\''))
			ss << "-i \"" << url << "\" ";
		else
			ss << "-i '" << url << "' ";

		// video output
		ss << "-f rawvideo ";
		ss << "-pix_fmt rgba ";
		ss << "-";

		if (!(video = popen(ss.str().c_str(), "r")))
			perror("popen");
	}
};
