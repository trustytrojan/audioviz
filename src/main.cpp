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

int main()
{
	sf::ContextSettings ctx;
	ctx.antialiasingLevel = 8;

	int sample_size = 3000;
	const auto audio_file = "../audioviz/Music/shotgun carousel.mp3";
	SndfileHandle sf(audio_file);
	std::vector<float> audio_buffer(sample_size * sf.channels());
	SpectrumDrawable sd(sample_size);

	const auto start_in_window = [&]
	{
		sf::RenderWindow window(sf::VideoMode({800, 600}), "audioviz-sfml", sf::Style::Titlebar, sf::State::Windowed, ctx);
		window.setVerticalSyncEnabled(true);

		PortAudio pa;
		auto pa_stream = pa.stream(0, sf.channels(), paFloat32, sf.samplerate(), sample_size);
		RenderStats rs;

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

		while (window.isOpen())
		{
			{
				const auto st = rs.createScopedTimer();
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
				window.clear();
				sd.draw(window, audio_buffer.data(), sf.channels(), 0, true);
				window.display();
			}
			sf.seek(avpvf - sample_size, SEEK_CUR);
			handle_events();
		}
	};

	// start_in_window();

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

		for (int frame = 0; frame < total_frames; ++frame)
		{
			const auto frames_read = sf.readf(audio_buffer.data(), sample_size);
			if (frames_read != sample_size)
				break;
			rt.clear();
			sd.draw(rt, audio_buffer.data(), sf.channels(), 0, true);
			rt.display();
			ff.encode(rt.getTexture().copyToImage());
			sf.seek(avpvf - sample_size, SEEK_CUR);
		}
	};

	encode_to_video();
}