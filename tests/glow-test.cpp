#include <SFML/Graphics.hpp>
#include <iostream>

/**
 * Minimal example of creating a glow around objects, with the added quirk of
 * using the same render-texture to draw on itself. I had been doing this in audioviz
 * for the longest time, and for months I never knew why AMD GPUs didn't like it.
 * (My main dev laptop is an Intel/NVIDIA hybrid.) Then some SFML folks told me it
 * was UB by OpenGL's terms. So I had to make sure of that with this test.
 */
int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <hrad> <vrad> <n_passes> [blur_only] [single_buffer]\n";
		return EXIT_FAILURE;
	}

	const float hrad = atof(argv[1]), vrad = atof(argv[2]);
	const int n_passes = atoi(argv[3]);
	const bool blur_only = argv[4] ? atoi(argv[4]) : false;
	const bool single_buffer = argv[5] ? atoi(argv[5]) : false;

	const sf::Vector2u size{400, 200};
	sf::RenderWindow window{
		sf::VideoMode{size}, "GlowTest", sf::Style::Titlebar, sf::State::Windowed, {.antiAliasingLevel = 4}};
	window.setVerticalSyncEnabled(true);

	sf::RectangleShape rect{{100, 100}};
	rect.setOrigin(rect.getGeometricCenter());
	rect.setPosition({100, 100});

	sf::CircleShape circ{50, 50};
	circ.setOrigin(circ.getGeometricCenter());
	circ.setPosition({300, 100});

	sf::RenderTexture rt{size}, rt2{size};
	sf::Sprite rt_spr{rt.getTexture()}, rt2_spr{rt2.getTexture()};

	sf::Shader blur{std::filesystem::path{"shaders/blur.frag"}, sf::Shader::Type::Fragment};
	blur.setUniform("size", sf::Glsl::Vec2{size});

	while (window.isOpen())
	{
		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();

		rt.clear();
		rt.draw(rect);
		rt.draw(circ);
		rt.display();

		for (int i = 0; i < n_passes; ++i)
		{
			blur.setUniform("direction", sf::Glsl::Vec2{hrad, 0});
			if (single_buffer)
				rt.draw(rt_spr, &blur);
			else
			{
				rt2.clear();
				rt2.draw(rt_spr, &blur);
				rt2.display();
				rt.draw(rt2_spr);
			}
			rt.display();

			blur.setUniform("direction", sf::Glsl::Vec2{0, vrad});
			if (single_buffer)
				rt.draw(rt_spr, &blur);
			else
			{
				rt2.clear();
				rt2.draw(rt_spr, &blur);
				rt2.display();
				rt.draw(rt2_spr);
			}
			rt.display();
		}

		window.clear();
		window.draw(rt_spr);
		if (!blur_only)
		{
			window.draw(rect);
			window.draw(circ);
		}
		window.display();
	}
}
