#include <SFML/Graphics.hpp>

void handleEvents(sf::RenderWindow &window)
{
	sf::Event event;
	while (window.pollEvent(event))
		switch (event.type)
		{
		case sf::Event::Closed:
			window.close();
			break;
			// case sf::Event::Resized:
			// 	// update the view to the new size of the window
			// 	sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
			// 	window.setView(sf::View(visibleArea));
			// 	break;
		}
}

void drawRectangleFromBottomLeft(sf::RenderTarget &rt, const int x, const int y, const int w, const int h, const sf::Color color)
{
	sf::RectangleShape rectangle(sf::Vector2f(w, h));
	rectangle.setPosition(x, y - h);
	rectangle.setFillColor(color);
	rt.draw(rectangle);
}

void drawCircleFromBottomLeft(sf::RenderTarget &rt, const int x, const int y, const int r, const sf::Color color)
{
	sf::CircleShape circle(r);
	circle.setPosition(x, y - 2 * r);
	circle.setFillColor(color);
	rt.draw(circle);
}

// (x, y) is the BOTTOM-LEFT corner
void drawVerticalPillFromBottomLeft(sf::RenderTarget &rt, const int x, const int y, const int w, const int h, const sf::Color color)
{
	const auto halfw = w / 2;
	drawCircleFromBottomLeft(rt, x, y, halfw, color);
	drawRectangleFromBottomLeft(rt, x, y - halfw, w, h, color);
	drawCircleFromBottomLeft(rt, x, y - h, halfw, color);
}

void drawLine(sf::RenderTarget &rt, const sf::Vector2f &start, const sf::Vector2f &end, const sf::Color color)
{
	const sf::Vertex vertices[] = {sf::Vertex(start, color), sf::Vertex(end, color)};
	rt.draw(vertices, 2, sf::Lines);
}

int main()
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Project", sf::Style::Close, settings);

	sf::RenderTexture renderTexture;
	if (!renderTexture.create(window.getSize().x, window.getSize().y))
		throw std::runtime_error("failed to create render texture!");

	sf::Shader shader;
	if (!shader.loadFromFile("simple.frag", sf::Shader::Fragment))
		throw std::runtime_error("failed to load shaders!");
	shader.setUniform("resolution", sf::Glsl::Vec2(window.getSize().x, window.getSize().y));

	sf::RenderStates rs(&shader);

	sf::Sprite sprite;

	while (window.isOpen())
	{
		handleEvents(window);
		window.clear();

		// start with a transparent background
		renderTexture.clear();

		drawVerticalPillFromBottomLeft(renderTexture, 200, 200, 20, 50, sf::Color::White);

		// finalize the drawing to the texture
		renderTexture.display();

		// create a sprite from the texture
		sprite.setTexture(renderTexture.getTexture());

		// draw the sprite to the window with the shader
		window.draw(sprite, rs);

		window.display();
	}
}