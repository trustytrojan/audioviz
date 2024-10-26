#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

int main()
{
	const sf::Vector2u window_size{500, 500};

	sf::RenderWindow window{sf::VideoMode{window_size}, "Rotating Vertices"};
	window.setVerticalSyncEnabled(true);

	const std::vector<sf::Vertex> va{
		{.position = {50, 0}, .color = {110, 80, 220}},
		{.position = {0, 100}, .color = {80, 50, 100}},
		{.position = {100, 100}, .color = {80, 50, 100}}};

	sf::Transformable tf;
	tf.setOrigin({50.f, 50.f});
	tf.setPosition({window_size.x / 2.f, window_size.y / 2.f});

	while (window.isOpen())
	{
		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();
			else if (const auto mm = event->getIf<sf::Event::MouseMoved>())
			{
				const auto mousePosition = window.mapPixelToCoords({mm->position.x, mm->position.y});
				const auto deltaPosition = mousePosition - tf.getPosition();
				const auto rotation = sf::radians(std::atan2(deltaPosition.y, deltaPosition.x)) + sf::degrees(90.f);
				tf.setRotation(rotation);
			}

		window.clear();
		window.draw(va.data(), va.size(), sf::PrimitiveType::Triangles, tf.getTransform());
		window.display();
	}
}
