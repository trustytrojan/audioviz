#include "Main.hpp"
#include "audioviz/media/FfmpegPopenMedia.hpp"

Main::Main(const int argc, const char *const *const argv)
	: args{argc, argv}
{
	const auto &size_args = args.get<std::vector<uint>>("--size");
	const sf::Vector2u size{size_args[0], size_args[1]};
	media = std::make_unique<audioviz::FfmpegPopenMedia>(args.get("media_url"), size);
	ttviz viz{size, *media, fa, cs, ss, ps};
	ps.set_rect({{}, (sf::Vector2i)size});
	use_args(viz);

	// --encode: render to video file
	switch (const auto &encode_args = args.get<std::vector<std::string>>("--encode"); encode_args.size())
	{
	case 0:
		viz.start_in_window(*media, "ttviz");
		break;
	case 1:
		viz.encode(*media, encode_args[0]);
		break;
	case 2:
		viz.encode(*media, encode_args[0], encode_args[1]);
		break;
	case 3:
		viz.encode(*media, encode_args[0], encode_args[1], encode_args[2]);
		break;
	default:
		throw std::logic_error{"--encode requires 1-3 arguments"};
	}
}
