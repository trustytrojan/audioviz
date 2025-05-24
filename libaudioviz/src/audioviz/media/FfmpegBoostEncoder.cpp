#include <audioviz/media/FfmpegBoostEncoder.hpp>
#include <audioviz/util.hpp>

namespace audioviz::media
{

FfmpegBoostEncoder::FfmpegBoostEncoder(
	const audioviz::Base &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
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
		if (vcodec.contains("vaapi"))
		{
			if (const auto vaapi_device = util::detect_vaapi_device(); !vaapi_device.empty())
			{
				args.insert(args.end(), {
					"-vaapi_device", vaapi_device,
					"-vf", "format=nv12,hwupload"
				});
			}
			else
				std::cerr << "failed to find a vaapi device for h264_vaapi ffmpeg encoder!\n";
		}
#endif
	// clang-format on

	args.emplace_back(outfile);

	std::cout << "encoder args: ";
	for (const auto &arg : args)
		std::cout << '\'' << arg << "' ";
	std::cout << '\n';

	c = bp::child{bp::search_path("ffmpeg"), args, bp::std_in<video_in, bp::std_err> stderr};
}

FfmpegBoostEncoder::~FfmpegBoostEncoder()
{
	video_in.close();
	c.wait();
}

void FfmpegBoostEncoder::send_frame(const sf::Image &img)
{
	const auto [x, y] = img.getSize();
	video_in.write(reinterpret_cast<const char *>(img.getPixelsPtr()), 4 * x * y);
}

} // namespace audioviz::media
