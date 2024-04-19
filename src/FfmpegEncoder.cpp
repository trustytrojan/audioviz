#include "FfmpegEncoder.hpp"
#include <iostream>

FfmpegEncoder::FfmpegEncoder(const sf::Vector2u &size, const std::string &output_file, const Options &options)
	: size(size)
{
	std::ostringstream ss;

	ss << '\'' << options.path
	   << "' -hide_banner -y -f rawvideo -pix_fmt rgba -s:v " << size.x << 'x' << size.y
	   << " -r " << options.fps
	   << " -i - ";

	if (options.audio_file.size())
		ss << " -i '" << options.audio_file << "' ";

	ss << " -c:v " << options.vcodec
	   << " -c:a " << options.acodec
	   << " -shortest '" << output_file << '\'';

	const auto command = ss.str();
	std::cout << command << '\n';
	ffmpeg = popen(command.c_str(), "w");
}

void FfmpegEncoder::encode(const sf::Image &img)
{
	if (img.getSize() != size)
		throw std::runtime_error("incompatible image size!");
	fwrite(img.getPixelsPtr(), 1, 4 * size.x * size.y, ffmpeg);
}
