#include "viz/ScopeDrawable.hpp"
#include <iostream>
#include <random>
#include "Media.hpp"

template <typename T>
static T random(const T min, const T max)
{
	static std::mt19937 gen(std::random_device{}());
	return std::uniform_real_distribution<>(min, max)(gen);
}

int main()
{
	const sf::Vector2u size{1920, 1080};
	sf::RenderWindow window{sf::VideoMode{size}, "ScopeDrawableTest"};

	viz::ScopeDrawable<sf::RectangleShape> scope;

	scope.set_rect({{}, (sf::Vector2i)size});
	scope.set_shape_spacing(0);
	scope.set_shape_width(1);
	scope.set_multiplier(200);

	Media media{"music/song.mp3"};
	media.init(size);

	std::vector<float> left_channel;
	left_channel.resize(1080);

	// scope.update_shape_positions(dummy_audio);
	int i = 0;
	while (window.isOpen())
	{
		while (const auto event = window.pollEvent())
		{
			// Check for the closed event
			if (event->is<sf::Event::Closed>())
				window.close(); // Close the window
		}

		window.clear();
		media.decode(1080);
		for (int i = 0; i < 1080; ++i)
			left_channel[i] = media.audio_buffer[i * media._astream.nb_channels() + 0];
		scope.update_shape_positions(media.audio_buffer);
		media.audio_buffer_erase(1080);
		window.draw(scope);
		
     
		window.display();
	}
}
