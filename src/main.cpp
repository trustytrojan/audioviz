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

	while (window.isOpen() && viz.draw_frame(window))
	{
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
	FILE *ffmpeg;

public:
	audioviz_encoder(sf::Vector2u size, const char *const url, const char *const out_url)
		: size(size),
		  viz(size, url)
	{
		std::ostringstream ss;

		// clang-format off
		ss << "ffmpeg -hide_banner -y"
			" -f rawvideo -pix_fmt rgba -s:v " << size.x << 'x' << size.y << " -r 60 -i -"
			" -i '" << url << "'"
			" -c:v h264 -c:a copy " << out_url;
		// clang-format on

		ffmpeg = popen(ss.str().c_str(), "w");
		setbuf(ffmpeg, NULL);

		viz.set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
		viz.set_metadata_position({30, 30});
	}

	void start()
	{
		MyRenderTexture rt(size, ctx);

		while (viz.draw_frame(rt))
		{
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

		while (viz.draw_frame(window))
		{
			window.display();
			txr.update(window);
			fwrite(txr.copyToImage().getPixelsPtr(), 1, 4 * size.x * size.y, ffmpeg);
		}
	}
};

int main(const int, const char *const *const argv)
{
	if (!argv[1])
	{
		std::cerr << "media url required\n";
		return EXIT_FAILURE;
	}

	try
	{
		if (argv[2] && !strcmp(argv[2], "--encode"))
			audioviz_encoder({1280, 720}, argv[1], "out.mp4").start();
		else
			play(argv[1]);
	}
	catch (const std::exception &e)
	{
		std::cerr << "audioviz: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
}
