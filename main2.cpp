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
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	const auto width = 800, height = 600;
	sf::RenderWindow window(sf::VideoMode(width, height), "SFML Project", sf::Style::Titlebar, settings);

	sf::Shader shader;
	if (!shader.loadFromFile("glow-pill.frag", sf::Shader::Fragment))
		throw std::runtime_error("failed to load shaders!");
	shader.setUniform("resolution", sf::Glsl::Vec2(width, height));
	sf::RenderStates rs(&shader);

	sf::RectangleShape r(sf::Vector2f(width, height));
	sf::Clock clock;

	while (window.isOpen())
	{
		handleEvents(window);
		window.clear();
		window.draw(r, rs);
		window.display();
    	std::cout << "\rFPS: " << (1.0f / clock.restart().asSeconds()) << std::flush; // Print the FPS
	}
}