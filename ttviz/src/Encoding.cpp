#include "Main.hpp"

#include <audioviz/media/FfmpegEncoder.hpp>

void Main::encode(audioviz::Base &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	FfmpegEncoder ffmpeg{viz, outfile, vcodec, acodec};

	if (enc_window)
	{
		sf::RenderWindow window{
			sf::VideoMode{viz.size},
			"audioviz - encoding...",
			sf::Style::Titlebar,
			sf::State::Windowed,
			{.antiAliasingLevel = 4},
		};
		sf::Texture txr{viz.size};
		while (viz.next_frame())
		{
			window.draw(viz);
			window.display();
			txr.update(window);
			ffmpeg.send_frame(txr);
		}
	}
	else
	{
		audioviz::RenderTexture rt{viz.size, 4};
		while (viz.next_frame())
		{
			rt.draw(viz);
			rt.display();
			ffmpeg.send_frame(rt.getTexture());
		}
	}
}
