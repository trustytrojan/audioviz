#include <GLFW/glfw3.h>
#include <iostream>
#include "audioviz.hpp"

// required to fix multiple definition errors
// will fix this eventually by making libavpp a compiled library
#include "../deps/libavpp/src/av/Util.cpp"

std::ostream &operator<<(std::ostream &ostr, const AVRational &r)
{
	return ostr << r.num << '/' << r.den;
}

static const sf::ContextSettings ctx(0, 0, 4);

void play(const char *const url)
{
	const sf::Vector2u size{1280, 720};

	audioviz viz(size, url);
	viz.set_audio_playback_enabled(true);
	viz.set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
	viz.set_metadata_position({30, 30});

	sf::RenderWindow window(sf::VideoMode(size), "audioviz", sf::Style::Titlebar, sf::State::Windowed, ctx);
	window.setVerticalSyncEnabled(true);

	{ // set framerate using display refresh rate (since we need vsync)
		const auto throwGlfwError = [](const std::string &func)
		{
			const char *errmsg;
			if (glfwGetError(&errmsg) != GLFW_NO_ERROR)
				throw std::runtime_error(func + errmsg);
		};
		if (!glfwInit())
			throwGlfwError("glfwInit");
		const auto monitor = glfwGetPrimaryMonitor();
		if (!monitor)
			throwGlfwError("glfwGetPrimaryMonitor");
		const auto mode = glfwGetVideoMode(monitor);
		if (!mode)
			throwGlfwError("glfwGetVideoMode");
		viz.set_framerate(mode->refreshRate);
		glfwTerminate();
	}

	// setting the mult for a cover art background, and applying it
	// (static backgrounds will not have effects applied automatically)
	viz.bg.effects.emplace_back(new fx::Mult{0.75}); // TODO: automate this based on total bg luminance!!!!!!!
	viz.bg.apply_fx();

	// change the default blur for a video (changing) background
	// viz.bg.effects[0] = std::make_unique<fx::Blur>(5, 5, 10);

	while (window.isOpen() && viz.prepare_frame())
	{
		window.draw(viz);
		window.display();
		while (const auto event = window.pollEvent())
		{
			if (event.is<sf::Event::Closed>())
				window.close();
		}
	}
}

class audioviz_encoder
{
	const sf::Vector2u size;
	audioviz viz;
	FILE *ffmpeg = nullptr;

	void ffmpeg_init(sf::Vector2u size, const std::string &url, const std::string &out_url)
	{
		char quote;
		const auto choose_quote = [&](const std::string &s)
		{
			quote = s.contains('\'') ? '"' : '\'';
		};

		std::ostringstream ss;

		// hide banner, overwrite existing output file
		ss << "ffmpeg -hide_banner -y ";

		// specify input 0: raw rgba video from stdin
		ss << "-f rawvideo -pix_fmt rgba -s:v " << size.x << 'x' << size.y << " -r 60 -i - ";

		// specify input 1: media file
		choose_quote(url);
		ss << "-ss -0.1 -i " << quote << url << quote << ' ';

		// only map the audio from input 1 to the output file
		ss << "-map 0 -map 1:a ";

		// specify video and audio encoder
		ss << "-c:v h264 -c:a copy ";

		// specify output file
		choose_quote(out_url);
		ss << quote << out_url << quote;

		const auto command = ss.str();
		std::cout << command << '\n';
		ffmpeg = popen(command.c_str(), "w");
		setbuf(ffmpeg, NULL);
	}

	void viz_init()
	{
		viz.set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
		viz.set_metadata_position({30, 30});

		viz.bg.effects.emplace_back(new fx::Mult{0.75}); // TODO: automate this based on total bg luminance!!!!!!!
		viz.bg.apply_fx();
	}

public:
	audioviz_encoder(sf::Vector2u size, const std::string &url, const std::string &out_url)
		: size(size),
		  viz(size, url)
	{
		ffmpeg_init(size, url, out_url);
		viz_init();
	}

	void start()
	{
		tt::RenderTexture rt(size, ctx);

		while (viz.prepare_frame())
		{
			rt.draw(viz);
			rt.display();
			fwrite(rt.getTexture().copyToImage().getPixelsPtr(), 1, 4 * size.x * size.y, ffmpeg);
		}
	}

	void start_window()
	{
		sf::RenderWindow window(sf::VideoMode(size), "encoder", sf::Style::Titlebar, sf::State::Windowed, ctx);
		sf::Texture txr;
		if (!txr.create(size))
			throw std::runtime_error("failed to create texture");

		while (viz.prepare_frame())
		{
			window.draw(viz);
			window.display();
			txr.update(window);
			fwrite(txr.copyToImage().getPixelsPtr(), 1, 4 * size.x * size.y, ffmpeg);
		}
	}
};

#include "Main.hpp"

int main(const int argc, const char *const *const argv)
{
	// if (!argv[1])
	// {
	// 	std::cerr << "media url required\n";
	// 	return EXIT_FAILURE;
	// }

	try
	{
		// if (argv[2] && !strcmp(argv[2], "--encode"))
		// {
		// 	if (!argv[3])
		// 		throw std::runtime_error("output filename required!");
		// 	audioviz_encoder({1280, 720}, argv[1], argv[3]).start();
		// }
		// else
		// 	play(argv[1]);
		Main(argc, argv);
	}
	catch (const std::exception &e)
	{
		std::cerr << "audioviz: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
}
