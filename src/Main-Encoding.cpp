#include "Main.hpp"

#include <future>
#include <queue>

#define future_not_finished(f) f.wait_for(std::chrono::seconds(0)) != std::future_status::ready

Main::FfmpegEncoder::FfmpegEncoder(audioviz &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
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
	_ss << "-ss -0.1 "; // THIS IS NECESSARY TO AVOID A/V DESYNC: starts muxing this input 0.1 seconds earlier than the other

#ifdef _WIN32
	_ss << "-i \"" << viz.get_media_url() << "\" ";
#else
	if (viz.get_media_url().contains('\''))
		_ss << "-i \"" << viz.get_media_url() << "\" ";
	else
		_ss << "-i '" << viz.get_media_url() << "' ";
#endif

	// specific stream mapping
	_ss << "-map 0 "; // use input 0
	_ss << "-map 1:a "; // only use the AUDIO stream of input 1 (in case it might also have a video stream)

	// specify encoders
	_ss << "-c:v " << vcodec << ' ';
	_ss << "-c:a " << acodec << ' ';

	// end on shortest input stream
	_ss << "-shortest ";

#ifdef LINUX
	if (vcodec.contains("vaapi"))
	{
		_ss << "-vaapi_device /dev/dri/render/D128 ";
		_ss << "-vf format=nv12,hwupload ";
	}
#endif

	// output file
	_ss << outfile;

	const auto &command = _ss.str();
	std::cout << command << '\n';
	process = popen(command.c_str(), "w");
}

Main::FfmpegEncoder::~FfmpegEncoder()
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

void Main::encode(audioviz &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	if (enc_window)
		encode_with_window(viz, outfile, vcodec, acodec);
	else
		encode_without_window(viz, outfile, vcodec, acodec);
}

void Main::encode_without_window(audioviz &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
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
void Main::encode_without_window_mt(audioviz &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	std::queue<sf::Image> images;

	const auto image_queuer = std::async(
		std::launch::async,
		[&]
		{
			tt::RenderTexture rt(viz.size, 4);
			while (viz.prepare_frame())
			{
				rt.draw(viz);
				rt.display();
				images.push(rt.getTexture().copyToImage());
				rt.clear();
			}
		});

	FfmpegEncoder ffmpeg{viz, outfile, vcodec, acodec};
	while (future_not_finished(image_queuer))
	{
		if (images.empty())
			continue;
		ffmpeg.send_frame(images.front());
		images.pop();
	}
}

void Main::encode_with_window(audioviz &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	FfmpegEncoder ffmpeg{viz, outfile, vcodec, acodec};
	sf::RenderWindow window{sf::VideoMode{viz.size}, "encoder", sf::Style::Titlebar, sf::State::Windowed, {.antiAliasingLevel = 4}};
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
