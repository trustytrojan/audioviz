#include <audioviz/media/FfmpegCliMedia.hpp>
#include <boost/log/trivial.hpp>

namespace audioviz::media
{

// try to fill _audio_buffer until it has samples_to_read floats.
// callers should check _audio_buffer.size() using audio_buffer_frames()
// to decide whether to stop processing.
void FfmpegCliMedia::decode_audio(const int frames)
{
	const auto samples_to_read = frames * _astream.nb_channels();
	BOOST_LOG_TRIVIAL(trace) << "frames=" << frames << " samples_to_read=" << samples_to_read << "\n";
	
	while (_audio_buffer.size() < samples_to_read)
	{
		float buf[samples_to_read];
		const auto samples_read = read_audio_samples(buf, samples_to_read);
		BOOST_LOG_TRIVIAL(trace) << "samples_read=" << samples_read << "\n";

		if (!samples_read)
		{
			BOOST_LOG_TRIVIAL(trace) << "no samples read, breaking loop\n";
			return;
		}

		_audio_buffer.insert(_audio_buffer.end(), buf, buf + samples_read);
	}
}

} // namespace media
