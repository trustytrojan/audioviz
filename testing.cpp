#include "include/PortAudio.hpp"
#include <stdexcept>
#include "AudioDecoder.cpp"
#include <sndfile.hh>

AudioDecoder *_decoder;
std::vector<float> audio_buf;

int pa_stream_callback(
	const void *input, void *output,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo *timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	*_decoder >> audio_buf;
	std::cout << audio_buf.size() << '\n';
	memcpy(output, audio_buf.data(), audio_buf.size() * sizeof(float));
	return paContinue;
}

void pacallbacktest()
{
	AudioDecoder decoder("Music/Feeel Goood.mp3");
	_decoder = &decoder;
	PortAudio _;
	PortAudio::Stream pa_stream(0, decoder.nb_channels(), paFloat32, decoder.sample_rate(), paFramesPerBufferUnspecified, pa_stream_callback);
	Pa_Sleep(decoder.duration_sec() * 1e3);
}

void pasynctest()
{
	AudioDecoder decoder("https://api.trustytrojan.dev/yt/dl/ta7gPUcxlrQ?only=audio");
	PortAudio _;
	PortAudio::Stream pa_stream(0, decoder.nb_channels(), paFloat32, decoder.sample_rate(), audio_buf.size());
	std::cout << decoder.duration_sec() << '\n';
	std::cout << decoder.frames() << '\n';
	while (decoder >> audio_buf)
	{
		// TODO: find out why this works!!!!!!!!!!
		pa_stream.write(audio_buf.data(), audio_buf.size() / decoder.nb_channels() / 4);
	}
}

void sndfiletest()
{
	SndfileHandle sf("Music/Feeel Goood.mp3");
	const size_t pa_frame_count = 30;
	PortAudio _;
	PortAudio::Stream pa_stream(0, sf.channels(), paFloat32, sf.samplerate(), pa_frame_count);
	auto audio_buf = new float[sf.channels() * pa_frame_count];
	while (sf.readf(audio_buf, pa_frame_count))
		pa_stream.write(audio_buf, pa_frame_count);
}

int main()
{
	pasynctest();
}
