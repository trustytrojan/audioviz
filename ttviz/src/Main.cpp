#include "Main.hpp"
#include <audioviz/Player.hpp>

Main::Main(const int argc, const char *const *const argv)
	: args{argc, argv}
{
	const auto &size_args = args.get<std::vector<uint>>("--size");
	const sf::Vector2u size{size_args[0], size_args[1]};
	const float start_time = args.get<float>("--start-time");

	audioviz::FfmpegPopenMedia media{args.get("media_url"), size, start_time};
	ttviz viz{size, media, args.get<uint>("-n")};

	const int framerate = args.get<uint>("-r");
	viz.set_framerate(args.get<uint>("-r"));

	viz.configure_from_args(args);

	// --encode: render to video file via Player
	audioviz::Player player{viz, media, framerate, viz.get_fa().get_fft_size()};
	switch (const auto &encode_args = args.get<std::vector<std::string>>("--encode"); encode_args.size())
	{
	case 0:
		player.start_in_window("ttviz");
		break;
	case 1:
		player.encode(encode_args[0], "", "");
		break;
	case 2:
		player.encode(encode_args[0], encode_args[1], "");
		break;
	case 3:
		player.encode(encode_args[0], encode_args[1], encode_args[2]);
		break;
	default:
		throw std::logic_error{"--encode requires 1-3 arguments"};
	}
}
