#include "Media3.hpp"

#include <iostream>

Media3::Media3(const std::string &url, const sf::Vector2u video_size)
	: url{url},
	  video_size{video_size},
	  _format{url}
{
	{ // read attached pic
		const auto &streams = _format.streams();
		if (const auto itr = std::ranges::find_if(
				streams, [](const auto &s) { return s->disposition & AV_DISPOSITION_ATTACHED_PIC; });
			itr != streams.cend())
		{
			const auto &stream = *itr;
			_attached_pic = {stream->attached_pic.data, stream->attached_pic.size};
		}
	}

	try // find video stream
	{
		const auto stream = _format.find_best_stream(AVMEDIA_TYPE_VIDEO);
		// we don't want to re-decode the attached pic stream
		if (!(stream->disposition & AV_DISPOSITION_ATTACHED_PIC))
			_vstream = stream;
	}
	catch (const av::Error &e)
	{
		std::cerr << e.what() << '\n';
	}

	{ // create audio decoder
		std::ostringstream ss;
		ss << "ffmpeg -v warning -hwaccel auto ";

		if (url.contains("http"))
			ss << "-reconnect 1 ";

#ifdef _WIN32
		ss << "-i \"" << url << "\" ";
#else
		if (url.contains('\''))
			ss << "-i \"" << url << "\" ";
		else
			ss << "-i '" << url << "' ";
#endif

		ss << "-f f32le - ";

		if (!(audio = popen(ss.str().c_str(), "r")))
			throw std::runtime_error{std::string{"popen: "} + strerror(errno)};
	}

	if (_vstream && video_size.x && video_size.y)
	{ // create video decoder
		std::ostringstream ss;
		ss << "ffmpeg ";
		ss << "-v warning ";
		ss << "-hwaccel auto ";

		if (url.contains("http"))
			ss << "-reconnect 1 ";

		if (url.contains('\''))
			ss << "-i \"" << url << "\" ";
		else
			ss << "-i '" << url << "' ";

// this needs more work!!!!!!!!!!
// need to determine whether vaapi works on the system before using vaapi
// also change quoting for windows

#ifdef LINUX
		ss << "-vaapi_device /dev/dri/renderD129 ";
		ss << "-vf 'format=nv12,hwupload,scale_vaapi=" << video_size.x << ':' << video_size.y << ",hwdownload' ";
#else
		ss << "-s " << video_size.x << 'x' << video_size.y << ' ';
#endif

		ss << "-pix_fmt rgba ";
		ss << "-f rawvideo ";
		ss << "-";

		if (!(video = popen(ss.str().c_str(), "r")))
			perror("popen");
	}
}

Media3::~Media3()
{
	if (audio && pclose(audio) == -1)
		perror("pclose");
	if (video && pclose(video) == -1)
		perror("pclose");
}

size_t Media3::read_audio_samples(float *const buf, const int samples) const
{
	if (!audio)
		throw std::logic_error{"no audio stream"};
	const auto samples_read = fread(buf, sizeof(float), samples, audio);
	if (const auto err = ferror(audio))
		std::cerr << "audio stream error: " << strerror(errno) << '\n';
	return samples_read;
}

bool Media3::read_video_frame(sf::Texture &txr) const
{
	if (!video)
		throw std::runtime_error{"no video stream available!"};
	if (!txr.resize(video_size))
		throw std::runtime_error{"texture resize failed!"};
	const auto bytes_to_read = 4 * video_size.x * video_size.y;
	uint8_t buf[bytes_to_read];
	if (fread(buf, sizeof(uint8_t), bytes_to_read, video) < bytes_to_read)
	{
		if (const auto err = ferror(video))
			std::cerr << "video error: " << strerror(err) << '\n';
		return false;
	}
	txr.update(buf);
	return true;
}

void Media3::decode_audio(const int frames)
{
	const auto samples_to_read = frames * _astream.nb_channels();
	while (_audio_buffer.size() < samples_to_read)
	{
		float buf[samples_to_read];
		const auto samples_read = read_audio_samples(buf, samples_to_read);
		std::cout << "read " << samples_read << " samples\n";
		// if (!samples_read)
			// return;
		_audio_buffer.insert(_audio_buffer.end(), buf, buf + samples_read);
	}
}

void Media3::audio_buffer_erase(const int frames)
{
	const auto begin = _audio_buffer.begin();
	const auto samples = frames * _astream.nb_channels();
	_audio_buffer.erase(begin, begin + samples);
}
