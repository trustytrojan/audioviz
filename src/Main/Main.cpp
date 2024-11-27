#include "Main.hpp"

Main::Main(const int argc, const char *const *const argv)
	: args{argc, argv}
{
#ifdef AUDIOVIZ_LUA
	if (const auto luafile = args.present("--luafile"))
	{
		// you MUST call script_file or safe_script_file, otherwise NO errors will be printed!
		LuaState(*this).script_file(*luafile);
		return;
		// lua environment is still in the works!!!!!!!!!
	}
#endif

	const auto &size_args = args.get<std::vector<uint>>("--size");
	const sf::Vector2u size{size_args[0], size_args[1]};
	audioviz viz{size, args.get("media_url"), fa, cs, ss, ps};
	ps.set_rect({{}, (sf::Vector2i)size});
	use_args(viz);

	// --encode: render to video file
	switch (const auto &encode_args = args.get<std::vector<std::string>>("--encode"); encode_args.size())
	{
	case 0:
		// default behavior: render to window
		start_in_window(viz);
		break;
	case 1:
		encode(viz, encode_args[0]);
		break;
	case 2:
		encode(viz, encode_args[0], encode_args[1]);
		break;
	case 3:
		encode(viz, encode_args[0], encode_args[1], encode_args[2]);
		break;
	default:
		throw std::logic_error("--encode requires 1-3 arguments");
	}
}

void Main::start_in_window(audioviz &viz)
{
#ifdef AUDIOVIZ_PORTAUDIO
	viz.set_audio_playback_enabled(true);
#endif

	sf::RenderWindow window{
		sf::VideoMode{viz.size},
		"audioviz",
		sf::State::Windowed,
		{.antiAliasingLevel = 4},
	};
	window.setVerticalSyncEnabled(!no_vsync);

	while (window.isOpen() && viz.prepare_frame())
	{
		window.draw(viz);
		window.display();
		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();
		window.clear();
	}
}
