#pragma once

#include <SFML/Graphics.hpp>
#include <boost/json.hpp>
#include <boost/process.hpp>

namespace bp = boost::process;
namespace bj = boost::json;

class Media2
{
	static std::string capture_stdout(const std::vector<std::string> &args)
	{
		if (args.empty())
			throw std::invalid_argument{"No command provided"};
		bp::ipstream pipe_stream;
		std::ostringstream output_stream;
		bp::child child{args, bp::std_out > pipe_stream};
		std::string line;
		while (pipe_stream >> line)
			output_stream << line << std::endl;
		child.wait();
		return output_stream.str();
	}

	static bj::object ffprobe_json(const std::string &url)
	{
		return bj::parse(capture_stdout(
							 {"ffprobe", "-v", "quiet", "-print_format", "json", "-show_format", "-show_streams", url}))
			.as_object();
	}

	bp::child c;
	bp::basic_pstream<float> audio;
	std::optional<bp::ipstream> video;
	bj::object format_meta, audio_meta, video_meta;

public:
	std::vector<float> audio_buffer;

	Media2(const std::string &url)
	{
		const auto meta = ffprobe_json(url);

		if (!meta.contains("format"))
			throw std::logic_error{"media file '" + url + "' has no format????"};
		format_meta = meta.at("format").as_object();

		if (!meta.contains("streams"))
			throw std::invalid_argument{"media file '" + url + "' has no streams!"};
		{
			const auto &streams = meta.at("streams").as_array();

			const auto &audio_itr =
				std::ranges::find_if(streams, [](auto &o) { return o.at("codec_type") == "audio"; });
			if (audio_itr == streams.end())
				throw std::invalid_argument{"media file '" + url + "' has no audio stream!"};
			audio_meta = audio_itr->as_object();

			const auto &video_itr =
				std::ranges::find_if(streams, [](auto &o) { return o.at("codec_type") == "video"; });
			if (video_itr != streams.end())
				video_meta = video_itr->as_object();
		}

		// clang-format off
		std::vector<std::string> args{
			"-v", "quiet",

			// input file
			"-hwaccel", "auto",
			"-i", url,

			// audio output
			"-f", "f32le",
			"-acodec", "pcm_f32le",
			"-"
		};

		if (!video_meta.empty())
		{
			video.emplace(bp::pipe{"audioviz_video"});
			args.insert(args.end(), {
				// video output
				"-f", "rawvideo",
				"-pix_fmt", "rgba",
				"pipe:audioviz_video",
			});
		}

		c = bp::child{bp::search_path("ffmpeg"), args,
			bp::std_in < bp::close,
			bp::std_out > audio};
		// clang-format on
	}

	~Media2()
	{
		c.wait();
		audio.pipe().close();
		audio.close();
		if (video)
		{
			video->pipe().close();
			video->close();
		}
	}

	void decode_audio(int num_frames)
	{
		float buf[num_frames];
		audio.readsome(buf, sizeof(buf));
		audio_buffer.insert(audio_buffer.end(), buf, buf + num_frames);
	}

	void decode_video_frame(sf::Texture &txr)
	{
		if (!video)
			throw std::logic_error{"no video stream available!"};
	}
};
