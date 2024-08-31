#include "Main.hpp"

#include <future>
#include <queue>

#define future_not_finished(f) f.wait_for(std::chrono::seconds(0)) != std::future_status::ready

// clang-format off
Main::FfmpegEncoder::FfmpegEncoder(audioviz &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	std::vector<std::string> args{
		bp::search_path("ffmpeg").string(),
		"-hide_banner", "-y",

		// input 0: raw frames from audioviz
		"-f", "rawvideo",
		"-pix_fmt", "rgba",
		"-s:v", std::to_string(viz.get_size().x) + 'x' + std::to_string(viz.get_size().y),
		"-r", std::to_string(viz.get_framerate()),
		"-i", "-",

		// input 1: audioviz's input media file
		"-ss", "-0.1", // THIS IS NECESSARY TO AVOID A/V DESYNC: starts muxing this input 0.1 seconds earlier than the other
		"-i", viz.get_media_url(),

		// specific stream mapping
		"-map", "0", // use input 0
		"-map", "1:a", // only use the AUDIO stream of input 1 (in case it might also have a video stream)

		// specify encoders
		"-c:v", vcodec,
		"-c:a", acodec,
		
		// end on shortest input stream
		"-shortest"};

	if (vcodec.contains("vaapi"))
	{
		// vaapi encoders require specifying a vaapi device and uploading to the gpu with nv12 pixel format
		// i've only tested this on a linux system with intel iris xe graphics
		args.emplace_back("-vaapi_device"); args.emplace_back("/dev/dri/renderD128");
		args.emplace_back("-vf"); 			args.emplace_back("format=nv12,hwupload");
	}

	// output file
	args.emplace_back(outfile);

	// pass arguments, allow writing to ffmpeg's stdin
	process = bp::child(args, bp::std_in < std_in);
}
// clang-format on

Main::FfmpegEncoder::~FfmpegEncoder()
{
	std_in.pipe().close();
	std_in.close();
	process.join();
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
	FfmpegEncoder ffmpeg(viz, outfile, vcodec, acodec);
	tt::RenderTexture rt(viz.get_size(), 4);
	while (viz.prepare_frame())
	{
		rt.draw(viz);
		rt.display();
		ffmpeg.std_in.write(rt.getTexture().copyToImage().getPixelsPtr(), 4 * viz.get_size().x * viz.get_size().y);
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
			tt::RenderTexture rt(viz.get_size(), 4);
			while (viz.prepare_frame())
			{
				rt.draw(viz);
				rt.display();
				images.push(rt.getTexture().copyToImage());
				rt.clear();
			}
		});

	FfmpegEncoder ffmpeg(viz, outfile, vcodec, acodec);
	while (future_not_finished(image_queuer))
	{
		if (images.empty())
			continue;
		ffmpeg.std_in.write(images.front().getPixelsPtr(), 4 * viz.get_size().x * viz.get_size().y);
		images.pop();
	}
}

void Main::encode_with_window(audioviz &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	FfmpegEncoder ffmpeg(viz, outfile, vcodec, acodec);
	sf::RenderWindow window(sf::VideoMode(viz.get_size()), "encoder", sf::Style::Titlebar, sf::State::Windowed, {.antialiasingLevel = 4});
	sf::Texture txr{viz.get_size()};
	while (viz.prepare_frame())
	{
		window.draw(viz);
		window.display();
		txr.update(window);
		ffmpeg.std_in.write(txr.copyToImage().getPixelsPtr(), 4 * viz.get_size().x * viz.get_size().y);
		window.clear();
	}
}
