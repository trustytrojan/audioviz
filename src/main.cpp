#include <GLFW/glfw3.h>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <sndfile.hh>
#include "PortAudio.hpp"
#include "RenderStats.hpp"
#include "SpectrumDrawable.hpp"
#include "FfmpegEncoder.hpp"

void throwGlfwError()
{
	const char *errmsg;
	if (glfwGetError(&errmsg) != GLFW_NO_ERROR)
		throw std::runtime_error(errmsg);
}

const sf::Sprite &toSprite(const sf::RenderTexture &rt)
{
	sf::Sprite sp(rt.getTexture());
	sp.setTexture(rt.getTexture());
	return sp;
}

struct MyRenderTexture : sf::RenderTexture
{
	MyRenderTexture(const sf::Vector2u &size, const sf::ContextSettings &settings = sf::ContextSettings())
	{
		if (!create(size, settings))
			throw std::runtime_error("failed to create render-texture!");
	}

	const sf::Sprite &getSprite()
	{
		// sf::Sprite constructor will only run once
		static sf::Sprite sp(getTexture());
		sp.setTexture(getTexture());
		return sp;
	}

	void blur(sf::Shader &shader, int hrad, int vrad, int npasses)
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
	sf::ContextSettings ctx;
	ctx.antialiasingLevel = 8;

	int sample_size = 3000;
	const auto audio_file = "../audioviz/Music/Trakhawk.mp3";
	SndfileHandle sf(audio_file);
	std::vector<float> audio_buffer(sample_size * sf.channels());
	SpectrumDrawable sd(sample_size);
	sf::Shader shader;
	if (!shader.loadFromFile("blur.frag", sf::Shader::Type::Fragment))
		throw std::runtime_error("failed to load shader!");

	const auto start_in_window = [&]
	{
		const sf::Vector2u size{1280, 720};

		sf::RenderWindow window(sf::VideoMode(size), "audioviz-sfml", sf::Style::Titlebar, sf::State::Windowed, ctx);
		window.setVerticalSyncEnabled(true);

		MyRenderTexture originalSpectrum(size, ctx), blurredSpectrum(size);

		PortAudio pa;
		auto pa_stream = pa.stream(0, sf.channels(), paFloat32, sf.samplerate(), sample_size);

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
		const int total_frames = sf.frames() / avpvf;

		sf::Color noAlpha{0, 0, 0, 0};
		RenderStats stats;
		stats.printHeader();

		sf::Texture bgTexture;
		if (!bgTexture.loadFromFile("../audioviz/media/urname-blurred.jpg"))
			throw std::runtime_error("failed to load background image!");
		sf::Sprite bgSprite(bgTexture);

		{ // make sure bgTexture fills up the whole screen, and is centered
			const auto tsize = bgTexture.getSize();
			bgSprite.setOrigin({tsize.x / 2, tsize.y / 2});
			bgSprite.setPosition({size.x / 2, size.y / 2});
			float scale = std::max((float)size.x / tsize.x, (float)size.y / tsize.y);
			bgSprite.setScale({scale, scale});
		}

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
				std::cerr << "Output underflowed\n";
			}
			if (frames_read != sample_size)
				break;

			// draw spectrum to original
			originalSpectrum.clear(noAlpha);
			sd.draw(originalSpectrum, audio_buffer.data(), sf.channels(), 0, true);
			originalSpectrum.display();

			// copy spectrum over
			blurredSpectrum.clear(noAlpha);
			blurredSpectrum.draw(originalSpectrum.getSprite());
			blurredSpectrum.display();

			// run h/v blur
			blurredSpectrum.blur(shader, 1, 1, 5);

			window.clear(noAlpha);
			window.draw(bgSprite);
			window.draw(blurredSpectrum.getSprite(), sf::BlendAdd);
			window.draw(originalSpectrum.getSprite());
			window.display();

			stats.updateAndPrint();

			sf.seek(avpvf - sample_size, SEEK_CUR);
			handle_events();
		}
	};

	const auto encode_to_video = [&]
	{
		sf::RenderTexture rt;
		if (!rt.create({1280, 720}, ctx))
			throw std::runtime_error("failed to create render-texture!");

		FfmpegEncoder::Options opts;
		opts.audio_file = audio_file;
		FfmpegEncoder ff({1280, 720}, "out.mp4", opts);

		// audio frames per video frame
		const auto avpvf = sf.samplerate() / opts.fps;
		const int total_frames = sf.frames() / avpvf;

		while (const auto frames_read = sf.readf(audio_buffer.data(), sample_size))
		{
			if (frames_read != sample_size)
				break;
			rt.clear();
			sd.draw(rt, audio_buffer.data(), sf.channels(), 0, true);
			rt.display();
			ff.encode(rt.getTexture().copyToImage());
			sf.seek(avpvf - sample_size, SEEK_CUR);
		}
	};

	start_in_window();
	// encode_to_video();
}