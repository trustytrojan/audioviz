#include <GLFW/glfw3.h>
#include <cmath>

#include "RenderStats.hpp"
#include "audioviz.hpp"

void play()
{
	const sf::Vector2u size{1600, 900};
	audioviz av(size, "Music/Purr Bass.mp3");

	Pa::PortAudio _;						// initialize portaudio library
	auto pa_stream = av.create_pa_stream(); // create PortAudio stream to play audio live

	// no need to provide context-settings for anti-aliasing
	// anti-aliasing is built in to audioviz
	sf::RenderWindow window(sf::VideoMode(size), "audioviz-sfml", sf::Style::Titlebar, sf::State::Windowed, sf::ContextSettings(0, 0, 4));

	// this is REQUIRED to ensure smooth playback
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
		av.set_framerate(mode->refreshRate);
		glfwTerminate();
	}

	av.set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
	av.set_background("images/purrbass.jpg", {{}, 0.75});
	av.set_album_cover("images/purrbass.jpg");

	// need to call after setting album cover to update text position
	// should find a way to remove this hassle
	av.set_metadata_position({30, 30});

	RenderStats stats;
	stats.printHeader();
	while (window.isOpen())
	{
		stats.restartClock();
		window.clear();
		if (!av.draw_frame(window, &pa_stream))
			break;
		window.display();
		stats.updateAndPrint();

		{ // handle events
			sf::Event event;
			while (window.pollEvent(event))
				switch (event.type)
				{
				case sf::Event::Closed:
					window.close();
				default:
					break;
				}
		}
	}
}

int main()
{
	try
	{
		play();
	}
	catch (const std::exception &e)
	{
		std::cerr << "audioviz: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
}
