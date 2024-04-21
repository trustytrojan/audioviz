#include <GLFW/glfw3.h>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <sndfile.hh>

#include "PortAudio.hpp"
#include "RenderStats.hpp"
#include "SpectrumDrawable.hpp"
#include "FfmpegEncoder.hpp"
#include "MyRenderTexture.hpp"

class audioviz
{
private:
	int sample_size = 3000;
	std::string audio_file;

	SndfileHandle sf = audio_file;
	std::vector<float> audio_buffer = std::vector<float>(sample_size * sf.channels());
	SpectrumDrawable sd = sample_size;
	sf::Shader blurShader;
	int framerate, afpvf;

	struct _bg
	{
		sf::Texture texture;
		sf::Sprite sprite;
		_bg() : sprite(texture) {}
	} bg;

	struct _spectrum
	{
		MyRenderTexture original, blurred;
		_spectrum(sf::Vector2u size, int antialiasing)
			: original(size, sf::ContextSettings(0, 0, antialiasing)),
			  blurred(size) {}
	} spectrum;

public:
	audioviz(sf::Vector2u size, std::string audio_file, int antialiasing = 4)
		: audio_file(audio_file),
		  spectrum(size, antialiasing)
	{
		if (!blurShader.loadFromFile("blur.frag", sf::Shader::Type::Fragment))
			throw std::runtime_error("failed to load blur shader: 'blur.frag' required in current directory");
	}

	void set_framerate(int framerate)
	{
		this->framerate = framerate;
		afpvf = sf.samplerate() / framerate;
	}

	void set_bg(const std::string &file)
	{
		if (!bg.texture.loadFromFile(file))
			throw std::runtime_error("failed to load background image: '" + file + '\'');
	}

	bool next_frame(sf::RenderTarget &target, Pa::Stream &pa_stream)
	{
		static const sf::Color zero_alpha{0, 0, 0, 0};
		static const auto margin = 5;
		const auto size = target.getSize();

		const auto frames_read = sf.readf(audio_buffer.data(), sample_size);
		if (!frames_read)
			return false;
		try
		{
			pa_stream.write(audio_buffer.data(), afpvf);
		}
		catch (const PortAudio::Error &e)
		{
			if (e.err != paOutputUnderflowed)
				throw;
		}
		if (frames_read != sample_size)
			return false;
		spectrum.original.clear(zero_alpha);
		if (sf.channels() == 2)
		{
			sd.copy_channel_to_input(audio_buffer.data(), sf.channels(), 0, true);
			sf::IntRect left_half{{margin, margin}, {size.x / 2 - 2 * margin + sd.bar.get_spacing() / 2, size.y - 2 * margin}};
			sd.draw(spectrum.original, left_half, true);
		}
	}

	void start_in_target(sf::RenderTarget &target, bool play_audio = true, unsigned antialiasing = 4)
	{
		const auto size = target.getSize();
		MyRenderTexture originalSpectrum(size, sf::ContextSettings(0, 0, antialiasing)), blurredSpectrum(size);

		// audio frames per video frame
		const auto afpvf = [this]
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
			int refresh_rate = video_mode->refreshRate;
			glfwTerminate();
			return sf.samplerate() / refresh_rate;
		}();

		std::optional<PortAudio> _;
		std::optional<PortAudio::Stream> pa_stream;
		if (play_audio)
		{
			_.emplace();
			pa_stream.emplace(0, sf.channels(), paFloat32, sf.samplerate(), afpvf);
		}

		sf::Color zeroAlpha{0, 0, 0, 0};
		RenderStats stats;
		stats.printHeader();

		{ // make sure background fills up the whole screen and is centered
			const auto tsize = bg.texture.getSize();
			bg.sprite.setOrigin({tsize.x / 2, tsize.y / 2});
			bg.sprite.setPosition({size.x / 2, size.y / 2});
			float scale = std::max((float)size.x / tsize.x, (float)size.y / tsize.y);
			bg.sprite.setScale({scale, scale});
		}
	}
};

int main()
{
	int sample_size = 3000;
	const auto audio_file = "Music/obsessed (feat. funeral).mp3";

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
		ctx.antialiasingLevel = 4;

		MyRenderTexture originalSpectrum(size, ctx), solidColorSpectrum(size), blurredSpectrum(size);

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
			int refresh_rate = video_mode->refreshRate;
			glfwTerminate();
			return refresh_rate;
		}();

		// audio frames per video frame
		const auto avpvf = sf.samplerate() / refresh_rate;

		sf::Color zeroAlpha{0, 0, 0, 0};
		RenderStats stats;
		stats.printHeader();

		sf::Texture bgTexture;
		if (!bgTexture.loadFromFile("images/obsessed-blurred.jpg"))
			throw std::runtime_error("failed to load background image!");
		sf::Sprite bgSprite(bgTexture);

		{ // make sure bgTexture fills up the whole screen, and is centered
			const auto tsize = bgTexture.getSize();
			bgSprite.setOrigin({tsize.x / 2, tsize.y / 2});
			bgSprite.setPosition({size.x / 2, size.y / 2});
			float scale = std::max((float)size.x / tsize.x, (float)size.y / tsize.y);
			bgSprite.setScale({scale, scale});
		}

		// pill testing
		// VerticalPill pill({50, 100});
		// pill.setPosition({500, 500});
		// while (window.isOpen())
		// {
		// 	window.clear();
		// 	window.draw(pill);
		// 	window.display();
		// 	handle_events();
		// }
		// return;

		const auto margin = 5;
		sd.bar.set_width(10);

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

			{ // draw the spectrum, one with the color wheel to be shown, one solid-color for the glow
				// clear with zero alpha, so that blur can be blended
				originalSpectrum.clear(zeroAlpha);
				// solidColorSpectrum.clear(zeroAlpha);

				// copy left channel to fftw input
				sd.copy_channel_to_input(audio_buffer.data(), sf.channels(), 0, true);

				// draw left-half spectrum
				sf::IntRect left_half{{margin, margin}, {size.x / 2 - 2 * margin + sd.bar.get_spacing() / 2, size.y - 2 * margin}};
				// sd.color.set_mode(SpectrumDrawable::ColorMode::WHEEL);
				sd.draw(originalSpectrum, left_half, true);

				// make a solid-color copy
				// sd.color.set_mode(SpectrumDrawable::ColorMode::SOLID);
				// sd.draw(solidColorSpectrum, left_half, true);

				// copy right channel to fftw input
				sd.copy_channel_to_input(audio_buffer.data(), sf.channels(), 1, true);

				// draw right-half spectrum
				sf::IntRect right_half{{size.x / 2 + sd.bar.get_spacing() / 2, margin}, {size.x / 2 - 2 * margin + sd.bar.get_spacing() / 2, size.y - 2 * margin}};
				// sd.color.set_mode(SpectrumDrawable::ColorMode::WHEEL);
				sd.draw(originalSpectrum, right_half);

				// make a solid-color copy
				// sd.color.set_mode(SpectrumDrawable::ColorMode::SOLID);
				// sd.draw(solidColorSpectrum, right_half);

				// save changes to internal textures
				originalSpectrum.display();
				// solidColorSpectrum.display();
			}

			// copy spectrum over
			blurredSpectrum.clear(zeroAlpha);
			blurredSpectrum.draw(originalSpectrum.sprite);
			blurredSpectrum.display();

			// blur the solid color spectrum, creating a glow to place behind the original spectrum
			blurredSpectrum.blur(shader, 0.5, 0.5, 5);

			window.draw(bgSprite);
			window.draw(blurredSpectrum.sprite, sf::BlendAdd);
			window.draw(originalSpectrum.sprite);

			// sf::RectangleShape rect1((sf::Vector2f)right_half.getSize()), rect2((sf::Vector2f)left_half.getSize());
			// rect1.setFillColor(noAlpha);
			// rect2.setFillColor(noAlpha);
			// rect1.setPosition((sf::Vector2f)right_half.getPosition());
			// rect2.setPosition((sf::Vector2f)left_half.getPosition());
			// rect1.setOutlineThickness(1);
			// rect2.setOutlineThickness(1);
			// window.draw(rect1);
			// window.draw(rect2);

			window.display();

			stats.updateAndPrint();

			sf.seek(avpvf - sample_size, SEEK_CUR);
			handle_events();
		}
	};

	const auto encode_to_video = [&] {

	};

	start_in_window();
	// encode_to_video();
}