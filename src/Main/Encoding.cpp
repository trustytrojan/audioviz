#include "Main.hpp"

#include <boost/process.hpp>
#include <future>
#include <queue>

namespace bp = boost::process;

#define future_not_finished(f) f.wait_for(std::chrono::seconds(0)) != std::future_status::ready

class FfmpegEncoder
{
public:
	static std::string detect_vaapi_device()
	{
		for (const auto &e : std::filesystem::directory_iterator("/dev/dri"))
		{
			const auto &path = e.path();
			if (!path.filename().string().starts_with("renderD"))
				continue;
			std::cerr << "detect_vaapi_device: testing " << path << '\n';
			// clang-format off
			bp::child c{
				bp::search_path("ffmpeg"),
				"-v", "warning",
				"-vaapi_device", path.string(),
				"-f", "lavfi", "-i", "testsrc=1280x720:d=1",
				"-vf", "format=nv12,hwupload,scale_vaapi=640:640",
				"-c:v", "h264_vaapi",
				"-f", "null", "-"};
			// clang-format on
			c.wait();
			if (c.exit_code() == 0)
			{
				std::cerr << "detect_vaapi_device: success, returning " << path << '\n';
				return path.string();
			}
		}
		std::cerr << "detect_vaapi_device: failed to find device\n";
		return {};
	}

private:
	bp::child c;
	bp::basic_pipe<const uint8_t> video_in;

public:
	FfmpegEncoder(audioviz &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
	{
		const auto &url = viz.get_media_url();

		// clang-format off
		std::vector<std::string> args{
			"-hide_banner",
			"-hwaccel", "auto",

			// input 0: raw video stream from audioviz
			"-f", "rawvideo",
			"-pix_fmt", "rgba",
			"-s:v", std::to_string(viz.size.x) + 'x' + std::to_string(viz.size.y),
			"-r", std::to_string(viz.get_framerate()),
			"-i", "-",

			// input 1: media used in audioviz
			"-ss", "-0.1" // THIS IS NECESSARY TO AVOID A/V DESYNC
		};

		// handle http media urls closing connections
		if (url.contains("http"))
			args.insert(args.end(), {"-reconnect", "1"});

		args.insert(args.end(), {
			"-i", url,
			// end of input 1 options

			// stream mapping
			"-map", "0",  // use input 0 (just a video stream)
			"-map", "1:a", // only use the audio from input 1 (which may have audio & video)

			// encoders
			"-c:v", vcodec,
			"-c:a", acodec,

			// end on shortest input stream
			"-shortest"
		});

#ifdef LINUX
		// if on linux and vaapi encoder used, detect a vaapi device for usage
		if (const auto vaapi_device = detect_vaapi_device(); vcodec.contains("vaapi") && !vaapi_device.empty())
			args.insert(args.end(), {
				"-vaapi_device", vaapi_device,
				"-vf", "format=nv12,hwupload"
			});
		else
			std::cerr << "failed to find a vaapi device for h264_vaapi ffmpeg encoder!\n";
#endif
		// clang-format on

		args.insert(args.end(), {"-y", outfile});
		c = bp::child{bp::search_path("ffmpeg"), args, bp::std_in < video_in};
	}

	~FfmpegEncoder()
	{
		video_in.close();
		c.wait();
	}

	void send_frame(const sf::Texture &txr)
	{
		send_frame(txr.copyToImage());
	}

	void send_frame(const sf::Image &img)
	{
		const auto [x, y] = img.getSize();
		video_in.write(img.getPixelsPtr(), 4 * x * y);
	}
};

/*
FfmpegEncoder::FfmpegEncoder(
	audioviz &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	std::ostringstream _ss;
	_ss << "ffmpeg -hide_banner -y ";

	// input 0: raw frames from audioviz
	_ss << "-f rawvideo ";
	_ss << "-pix_fmt rgba ";
	_ss << "-s:v " << viz.size.x << 'x' << viz.size.y << ' ';
	_ss << "-r " << viz.get_framerate() << ' ';
	_ss << "-i - ";

	// input 1: audioviz's input media file
	_ss << "-ss -0.1 "; // THIS IS NECESSARY TO AVOID A/V DESYNC
						// starts muxing this input 0.1 seconds earlier than the other

#ifdef _WIN32
	_ss << "-i \"" << viz.get_media_url() << "\" ";
#else
	if (viz.get_media_url().contains('\''))
		_ss << "-i \"" << viz.get_media_url() << "\" ";
	else
		_ss << "-i '" << viz.get_media_url() << "' ";
#endif

	// specific stream mapping
	_ss << "-map 0 ";	// use input 0
	_ss << "-map 1:a "; // only use the AUDIO stream of input 1 (in case it might also have a video stream)

	// specify encoders
	_ss << "-c:v " << vcodec << ' ';
	_ss << "-c:a " << acodec << ' ';

	// end on shortest input stream
	_ss << "-shortest ";

#ifdef LINUX
	if (vcodec.contains("vaapi"))
	{
		_ss << "-vaapi_device /dev/dri/renderD129 ";
		_ss << "-vf 'format=nv12,hwupload' ";
	}
#endif

	// output file
#ifdef _WIN32
	_ss << '"' << outfile << "\" ";
#else
	if (outfile.contains('\''))
		_ss << '"' << outfile << "\" ";
	else
		_ss << '\'' << outfile << "' ";
#endif

	const auto &command = _ss.str();
	std::cout << command << '\n';
	process = popen(command.c_str(), "w");
}

FfmpegEncoder::~FfmpegEncoder()
{
	if (pclose(process) == -1)
		perror("pclose");
}

void Main::FfmpegEncoder::send_frame(const sf::Texture &txr)
{
	send_frame(txr.copyToImage());
}

void Main::FfmpegEncoder::send_frame(const sf::Image &img)
{
	const auto [x, y] = img.getSize();
	fwrite(img.getPixelsPtr(), 4 * x * y, 1, process);
}
*/

void Main::encode(audioviz &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	if (enc_window)
		encode_with_window(viz, outfile, vcodec, acodec);
	else
		encode_without_window(viz, outfile, vcodec, acodec);
}

void Main::encode_without_window(
	audioviz &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	FfmpegEncoder ffmpeg{viz, outfile, vcodec, acodec};
	tt::RenderTexture rt{viz.size, 4};
	while (viz.prepare_frame())
	{
		rt.draw(viz);
		rt.display();
		ffmpeg.send_frame(rt.getTexture());
		rt.clear();
	}
}

// multi-threaded implementation; has not benchmarked against single-threaded yet.
// single-threaded will be preferred until then.
void Main::encode_without_window_mt(
	audioviz &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	std::queue<sf::Image> images;

	// clang-format off
	const auto image_queuer = std::async(std::launch::async, [&]
	{
		tt::RenderTexture rt{viz.size, 4};
		while (viz.prepare_frame())
		{
			rt.draw(viz);
			rt.display();
			images.push(rt.getTexture().copyToImage());
			rt.clear();
		}
	});
	// clang-format on

	FfmpegEncoder ffmpeg{viz, outfile, vcodec, acodec};
	while (future_not_finished(image_queuer))
	{
		if (images.empty())
			continue;
		ffmpeg.send_frame(images.front());
		images.pop();
	}
}

void Main::encode_with_window(
	audioviz &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	FfmpegEncoder ffmpeg{viz, outfile, vcodec, acodec};
	sf::RenderWindow window{
		sf::VideoMode{viz.size},
		"encoder",
		sf::Style::Titlebar,
		sf::State::Windowed,
		{.antiAliasingLevel = 4},
	};
	sf::Texture txr{viz.size};
	while (viz.prepare_frame())
	{
		window.draw(viz);
		window.display();
		txr.update(window);
		ffmpeg.send_frame(txr);
		window.clear();
	}
}
