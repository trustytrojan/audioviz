#include "Main.hpp"

#include <boost/process.hpp>

namespace bp = boost::process;

class FfmpegEncoder
{
public:
#ifdef LINUX
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
#endif

private:
	bp::child c;
	bp::basic_pipe<uint8_t> video_in;

public:
	FfmpegEncoder(const audioviz::Base &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
	{
		const auto &url = viz.get_media_url();

		// clang-format off
		std::vector<std::string> args{
			"-hide_banner", "-y", // THE -y NEEDS TO BE AT THE BEGINNING ON WINDOWS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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

		args.emplace_back(outfile);

		std::cerr << "encoder args: ";
		for (const auto &arg : args)
			std::cerr << '\'' << arg << "' ";
		std::cerr << '\n';

		c = bp::child{bp::search_path("ffmpeg"), args, bp::std_in<video_in, bp::std_err> stderr};
	}

	~FfmpegEncoder()
	{
		video_in.close();
		c.wait();
	}

	void send_frame(const sf::Texture &txr) { send_frame(txr.copyToImage()); }

	void send_frame(const sf::Image &img)
	{
		const auto [x, y] = img.getSize();
		video_in.write(img.getPixelsPtr(), 4 * x * y);
	}
};

void Main::encode(audioviz::Base &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	FfmpegEncoder ffmpeg{viz, outfile, vcodec, acodec};

	if (enc_window)
	{
		sf::RenderWindow window{
			sf::VideoMode{viz.size},
			"audioviz - encoding...",
			sf::Style::Titlebar,
			sf::State::Windowed,
			{.antiAliasingLevel = 4},
		};
		sf::Texture txr{viz.size};
		while (viz.next_frame())
		{
			window.draw(viz);
			window.display();
			txr.update(window);
			ffmpeg.send_frame(txr);
			// window.clear();
		}
	}
	else
	{
		audioviz::RenderTexture rt{viz.size, 4};
		while (viz.next_frame())
		{
			rt.draw(viz);
			rt.display();
			ffmpeg.send_frame(rt.getTexture());
			// rt.clear();
		}
	}
}
