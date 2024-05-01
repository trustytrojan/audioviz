#include "include/av/MediaFileReader.hpp"
#include "include/av/StreamDecoder.hpp"
#include "include/av/Resampler.hpp"
#include "include/av/Vector.hpp"
#include <iostream>

int main()
{
	av::MediaFileReader format("Music/murderevery1.opus");
	std::cout << format->nb_streams << "\n\n";
	for (const auto &stream : format.streams())
		std::cout << stream->index << '\n'
				  << avcodec_get_name(stream->codecpar->codec_id) << '\n'
				  << stream.duration_sec() << '\n'
				  << stream.metadata("comment") << '\n'
				  << (void *)stream->attached_pic.data << "\n\n";
}