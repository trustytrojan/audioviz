#include <SFML/Graphics.hpp>

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
void drawPillFromBottomLeft(sf::RenderTarget &rt, const int x, const int y, const int w, const int h, const sf::Color color)
{
	const auto halfw = w / 2;
	drawCircleFromBottomLeft(rt, x, y, halfw, color);
	drawRectangleFromBottomLeft(rt, x, y - halfw, w, h, color);
	drawCircleFromBottomLeft(rt, x, y - h, halfw, color);
}

void drawLine(sf::RenderTarget &rt, const sf::Vector2f start, const sf::Vector2f end, const sf::Color color)
{
	const sf::Vertex vertices[] = {sf::Vertex(start, color), sf::Vertex(end, color)};
	rt.draw(vertices, 2, sf::Lines);
}

int main()
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	const auto width = 800, height = 600;
	sf::RenderWindow window(sf::VideoMode(width, height), "SFML Project", sf::Style::Titlebar, settings);

	sf::RenderTexture rt;
	if (!rt.create(width, height))
		throw std::runtime_error("failed to create render texture!");

	sf::Shader shader;
	if (!shader.loadFromFile("glow.frag", sf::Shader::Fragment))
		throw std::runtime_error("failed to load shaders!");
	shader.setUniform("resolution", sf::Glsl::Vec2(width, height));
	shader.setUniform("glowColor", sf::Glsl::Vec3(1, 1, 1));

	sf::RenderStates rs(&shader);
	sf::Sprite rtSprite(rt.getTexture());

	while (window.isOpen())
	{
		handleEvents(window);
		window.clear();

		// render scene to texture
		rt.clear({0, 0, 0, 0});
		drawPillFromBottomLeft(rt, 200, 200, 20, 50, sf::Color::Red);
		drawPillFromBottomLeft(rt, 300, 200, 20, 50, sf::Color::Red);
		drawPillFromBottomLeft(rt, 250, 250, 20, 50, sf::Color::Red);
		rt.display();

		// apply glow on texture several times, drawing the result to the window
		for (float i = 0; i < 2; i += 0.5f)
		{
			// glow vertically
			shader.setUniform("direction", sf::Glsl::Vec2(0, i));
			window.draw(rtSprite, rs);

			// glow horizontally
			shader.setUniform("direction", sf::Glsl::Vec2(i, 0));
			window.draw(rtSprite, rs);

			float j = i / 1.5;

			// glow on the up-diagonal
			shader.setUniform("direction", sf::Glsl::Vec2(j, j));
			window.draw(rtSprite, rs);

			// glow on the down-diagonal
			shader.setUniform("direction", sf::Glsl::Vec2(j, -j));
			window.draw(rtSprite, rs);
		}

		// overlap the glow results with the original scene
		window.draw(rtSprite);

		window.display();
	}
}