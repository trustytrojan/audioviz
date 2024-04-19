#include <SFML/Graphics.hpp>

class FfmpegEncoder
{
	FILE *ffmpeg;
	sf::Vector2u size;

public:
	struct Options
	{
		int fps = 60;
		std::string path = "ffmpeg", vcodec = "h264", acodec = "copy", audio_file;
	};

	FfmpegEncoder(const sf::Vector2u &size, const std::string &output_file, const Options &options);
	void encode(const sf::Image &img);
};