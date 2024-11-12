#include "viz/ScopeDrawable.hpp"
#include "Media.hpp"
#include "tt/FrequencyAnalyzer.hpp"
#include "viz/SpectrumDrawable.hpp"
#include "viz/VerticalBar.hpp"
#include <cmath>
#include <iostream>
#include <portaudio.hpp>

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{atoi(argv[1]), atoi(argv[2])};
	sf::RenderWindow window{sf::VideoMode{size}, "ScopeDrawableTest"};
	window.setVerticalSyncEnabled(true);

	viz::ScopeDrawable<sf::RectangleShape> scope{{{}, (sf::Vector2i)size}};
	scope.set_shape_spacing(0);
	scope.set_shape_width(1);
	scope.set_fill_in(false);

	viz::SpectrumDrawable<viz::VerticalBar> sd;
	sd.set_rect({{}, (sf::Vector2i)size});
	sd.set_bar_width(1);
	sd.set_bar_spacing(0);
	sd.set_color_mode(viz::SpectrumDrawable<viz::VerticalBar>::ColorMode::WHEEL);
	sd.set_color_wheel_rate(0.005);

	const auto fft_size = size.x;
	tt::FrequencyAnalyzer fa{fft_size};

	Media media{argv[3]};
	media.init(size);

	int afpvf{media._astream.sample_rate() / 60};

	std::vector<float> left_channel(size.x), spectrum(fft_size);

	pa::PortAudio _;
	pa::Stream pa_stream{0, media._astream.nb_channels(), paFloat32, media._astream.sample_rate()};
	pa_stream.start();

	const sf::Vector2f _origin{size.x / 2.f, size.y / 2.f};

	const auto rad = 15;
	sf::CircleShape origincircle{rad};
	origincircle.setFillColor(sf::Color::Red);
	origincircle.setOrigin({rad / 2.f, rad / 2.f});

	double cur = 0;

	origincircle.setPosition(_origin);

	while (window.isOpen())
	{
		cur += .1;
		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();

		{
			media.decode(size.x);

			if (media.audio_buffer.size() < size.x)
				break;

			// copy just the left channel
			for (int i = 0; i < size.x; ++i)
				left_channel[i] = media.audio_buffer[i * media._astream.nb_channels() + 0 /* left channel */];
			scope.update_shape_positions(left_channel);

			fa.copy_to_input(left_channel.data());
			fa.render(spectrum);
			sd.update_bar_heights(spectrum);
			sd.color_wheel_increment();

			try
			{
				pa_stream.write(media.audio_buffer.data(), afpvf);
			}
			catch (const pa::Error &e)
			{
				if (e.code != paOutputUnderflowed)
					throw;
				std::cerr << e.what() << '\n';
			}
			media.audio_buffer_erase(afpvf);
		}
		double spin_arc = 360;
		double speed = 20;

		// tf.setRotation(sf::degrees(spin_arc*sin(cur/speed)));
		// tf.setRotation(sf::degrees(speed*sin(cur) + speed*cur));
		// tf.setRotation(sf::degrees(exp(cur/5 + 2*sin(cur))));
		// tf.setRotation(sf::degrees(2*sin(cur)*exp(2*sin(cur))));
		//float max_channel = 30 * (*std::max_element(left_channel.begin(), left_channel.end()));
		// tf.setRotation(5*sf::degrees(max_channel));
		sf::Angle ang_deg = sf::degrees(-7*cur);
		sf::Angle ang_deg2 = sf::degrees(90+7*cur);
		
		scope.set_rotation_angle(ang_deg);
		sf::Vector2f coord {150,20};

		scope.set_center_point(150, ang_deg2);

		window.clear();
		window.draw(scope);
		// window.draw(sd, tf.getTransform());
		window.draw(origincircle);
		window.display();
	}
}
