#include <GLFW/glfw3.h>
#include <cmath>

#include "RenderStats.hpp"
#include "FfmpegEncoder.hpp"
#include "audioviz.hpp"

int main()
{
	const sf::Vector2u size{1280, 720};

	PortAudio _; // initialize portaudio
	audioviz av(size, "Music/Eest.mp3");
	auto pa_stream = av.create_pa_stream();

	sf::RenderWindow window(sf::VideoMode(size), "audioviz-sfml", sf::Style::Titlebar, sf::State::Windowed, sf::ContextSettings(0, 0, 4));
	window.setVerticalSyncEnabled(true);

	const auto handle_events = [&]
	{
		sf::Event event;
		while (window.pollEvent(event))
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
			default:
				break;
			}
	};

	{ // set framerate using display refresh rate
		const auto throwGlfwError = []
		{
			const char *errmsg;
			if (glfwGetError(&errmsg) != GLFW_NO_ERROR)
				throw std::runtime_error(errmsg);
		};
		if (!glfwInit())
			throwGlfwError();
		const auto monitor = glfwGetPrimaryMonitor();
		if (!monitor)
			throwGlfwError();
		const auto video_mode = glfwGetVideoMode(monitor);
		if (!video_mode)
			throwGlfwError();
		av.set_framerate(video_mode->refreshRate);
		glfwTerminate();
	}

	av.set_bg("images/feelgood-blurred-10.jpg");

	while (window.isOpen())
	{
		window.clear();
		if (!av.draw_frame(window, &pa_stream))
			break;
		window.display();
		handle_events();
	}
}
