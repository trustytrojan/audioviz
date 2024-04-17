#include <SFML/Graphics.hpp>
#include <iostream>

void handleEvents(sf::RenderWindow &window)
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
}

int main()
{
	const auto width = 1000, height = 1000;
	sf::RenderWindow window(sf::VideoMode(width, height), "SFML Project", sf::Style::Titlebar);
	// window.setVerticalSyncEnabled(true);

	sf::Shader shader;
	if (!shader.loadFromFile("glow-pill-460.frag", sf::Shader::Fragment))
		throw std::runtime_error("failed to load shaders!");
	shader.setUniform("resolution", sf::Glsl::Vec2{width, height});
	sf::RenderStates rs(&shader);
	sf::RectangleShape r({width, height});
	sf::Clock clock;

	while (window.isOpen())
	{
		handleEvents(window);
		window.clear();
		window.draw(r, rs);
		window.display();
		const auto time = clock.restart();
		std::cout << "\e[2K\e[1A\e[2K"
				  << "Draw time: " << time.asMilliseconds() << "ms\n"
				  << "FPS: " << (1 / time.asSeconds()) << '\r'
				  << std::flush;
	}

	std::cout << '\n';
}