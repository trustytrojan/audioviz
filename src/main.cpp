#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include "audioviz.hpp"

void play(const char *const url)
{
	const sf::Vector2u size{1280, 720};
	audioviz av(size, url);

	// enable audio playback using portaudio
	av.set_audio_playback_enabled(true);

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

	// TODO: add separate method for applying effects to the background

	// need to call after setting album cover to update text position
	// should find a way to remove this hassle
	av.set_metadata_position({30, 30});

	while (window.isOpen())
	{
		window.clear();
		if (!av.draw_frame(window))
			break;
		window.display();
		while (const auto event = window.pollEvent())
			if (event.is<sf::Event::Closed>())
				window.close();
	}
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 2 || !argv[1] || !*argv[1])
	{
		std::cerr << "media url required\n";
		return EXIT_FAILURE;
	}

	try
	{
		play(argv[1]);
	}
	catch (const std::exception &e)
	{
		std::cerr << "audioviz: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
}
