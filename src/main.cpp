#include <GLFW/glfw3.h>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <sndfile.hh>
#include "PortAudio.hpp"
#include "RenderStats.hpp"
#include "SpectrumDrawable.hpp"
#include "FfmpegEncoder.hpp"

struct MyRenderTexture : sf::RenderTexture
{
	sf::Sprite sprite;

	MyRenderTexture(const sf::Vector2u &size, const sf::ContextSettings &settings = sf::ContextSettings())
		: sprite(getTexture())
	{
		if (!create(size, settings))
			throw std::runtime_error("failed to create render-texture!");
	}

	void display()
	{
		sf::RenderTexture::display();
		sprite.setTexture(getTexture(), true);
	}

	const sf::Sprite &getSprite()
	{
		// sf::Sprite constructor will only run once
		static sf::Sprite sp(getTexture());
		sp.setTexture(getTexture());
		return sp;
	}

	void blur(sf::Shader &shader, float hrad, float vrad, int npasses)
	{
		for (int i = 0; i < npasses; ++i)
		{
			shader.setUniform("direction", sf::Glsl::Vec2{hrad, 0}); // horizontal blur
			draw(getSprite(), &shader);
			display();

			shader.setUniform("direction", sf::Glsl::Vec2{0, vrad}); // vertical blur
			draw(getSprite(), &shader);
			display();
		}
	}
};

int main()
{
	int sample_size = 3000;
	const auto audio_file = "Music/Trakhawk.mp3";

	SndfileHandle sf(audio_file);
	std::vector<float> audio_buffer(sample_size * sf.channels());
	SpectrumDrawable sd(sample_size);

	sf::Shader shader;
	if (!shader.loadFromFile("blur.frag", sf::Shader::Type::Fragment))
		throw std::runtime_error("failed to load shader!");

	const auto start_in_window = [&]
	{
		const sf::Vector2u size{1280, 720};

		sf::RenderWindow window(sf::VideoMode(size), "audioviz-sfml", sf::Style::Titlebar, sf::State::Windowed);
		window.setVerticalSyncEnabled(true);

		// we only want antialiasing on the spectrum to round off the pills
		sf::ContextSettings ctx;
		ctx.antialiasingLevel = 16;

		MyRenderTexture originalSpectrum(size, ctx), blurredSpectrum(size);

		PortAudio _; // initializes portaudio
		Pa::Stream pa_stream(0, sf.channels(), paFloat32, sf.samplerate(), sample_size);

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

		const auto refresh_rate = []
		{
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
			glfwTerminate();
			return video_mode->refreshRate;
		}();

		// audio frames per video frame
		const auto avpvf = sf.samplerate() / refresh_rate;

		sf::Color noAlpha{0, 0, 0, 0};
		RenderStats stats;
		stats.printHeader();

		sf::Texture bgTexture;
		if (!bgTexture.loadFromFile("media/ischezayu-blurred.jpg"))
			throw std::runtime_error("failed to load background image!");
		sf::Sprite bgSprite(bgTexture);

		{ // make sure bgTexture fills up the whole screen, and is centered
			const auto tsize = bgTexture.getSize();
			bgSprite.setOrigin({tsize.x / 2, tsize.y / 2});
			bgSprite.setPosition({size.x / 2, size.y / 2});
			float scale = std::max((float)size.x / tsize.x, (float)size.y / tsize.y);
			bgSprite.setScale({scale, scale});
		}

		const auto margin = 5;
		sd.bar.set_width(15);

		while (window.isOpen())
		{
			stats.restartClock();

			const auto frames_read = sf.readf(audio_buffer.data(), sample_size);
			if (!frames_read)
				break;
			try
			{
				pa_stream.write(audio_buffer.data(), avpvf);
			}
			catch (const PortAudio::Error &e)
			{
				if (e.err != paOutputUnderflowed)
					throw;
			}
			if (frames_read != sample_size)
				break;

			// draw spectrum to texture
			originalSpectrum.clear(noAlpha);
			sd.copy_channel_to_input(audio_buffer.data(), sf.channels(), 0, true);
			sd.draw(originalSpectrum, {{size.x / 2, margin}, {size.x / 2, size.y - 5}});
			sd.copy_channel_to_input(audio_buffer.data(), sf.channels(), 1, true);
			sd.draw(originalSpectrum, {{margin, margin}, {size.x / 2, size.y - 5}}, true);
			originalSpectrum.display();

			// copy spectrum over
			blurredSpectrum.clear(noAlpha);
			blurredSpectrum.draw(originalSpectrum.getSprite());
			blurredSpectrum.display();

			// run h/v blur
			blurredSpectrum.blur(shader, 0.5, 0.5, 5);

			window.draw(bgSprite);
			window.draw(blurredSpectrum.getSprite(), sf::BlendAdd);
			window.draw(originalSpectrum.sprite);
			window.display();

			stats.updateAndPrint();

			sf.seek(avpvf - sample_size, SEEK_CUR);
			handle_events();
		}
	};

	const auto encode_to_video = [&]
	{

	};

	start_in_window();
	// encode_to_video();
}