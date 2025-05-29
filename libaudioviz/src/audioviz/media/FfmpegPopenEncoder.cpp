#include "_deps/sfml-src/extlibs/headers/glad/include/glad/gl.h"
#include "_deps/sfml-src/include/SFML/Window/GlResource.hpp"
#include <audioviz/media/FfmpegPopenEncoder.hpp>
#include <audioviz/util.hpp>
#include <cstring>
#include <stdexcept>

#ifdef _WIN32
#define POPEN_MODE "wb"
#elifdef __unix__
#define POPEN_MODE "w"
#endif

namespace audioviz
{

FfmpegPopenEncoder::FfmpegPopenEncoder(
	const audioviz::Base &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	// pbos = {0};
	video_size = viz.size;
	glGenBuffers(NUM_PBOS, pbos);

	const unsigned int byte_size = viz.size.x * viz.size.y * 4; // 4 refers to channel count
	for (unsigned int pbo : pbos) {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
		glBufferData(GL_PIXEL_PACK_BUFFER, byte_size, nullptr, GL_STREAM_READ);
	}
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);



	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);


	const auto &url = viz.get_media_url();
	std::ostringstream cmd_stream;
	cmd_stream << "ffmpeg -hide_banner -hwaccel auto -y ";

	// input 0: raw video stream from audioviz
	cmd_stream << "-f rawvideo -pix_fmt rgba "
			   << "-s " << viz.size.x << "x" << viz.size.y << " "
			   << "-r " << viz.get_framerate() << " "
			   << "-i - ";

	// input 1: media used in audioviz
	cmd_stream << "-ss -0.1 ";
	if (url.find("http") != std::string::npos)
		cmd_stream << "-reconnect 1 ";
	cmd_stream << "-i \"" << url << "\" ";

#ifdef __linux__
	// if on linux and vaapi encoder used, detect a vaapi device for usage
	if (vcodec.find("vaapi") != std::string::npos)
	{
		if (const auto vaapi_device = util::detect_vaapi_device(); !vaapi_device.empty())
			cmd_stream << "-vaapi_device " << vaapi_device << " -vf format=nv12,hwupload ";
		else
			std::cerr << "failed to find a vaapi device for h264_vaapi ffmpeg encoder!\n";
	}
#endif

	// stream mapping
	cmd_stream << "-map 0 -map 1:a ";

	// encoders
	cmd_stream << "-c:v " << vcodec << " -c:a " << acodec << " ";

	// end on shortest input stream
	cmd_stream << "-shortest " << outfile;

	ffmpeg = popen(cmd_stream.str().c_str(), POPEN_MODE);
	if (!ffmpeg)
		throw std::runtime_error("Failed to start ffmpeg process with popen");
}

FfmpegPopenEncoder::~FfmpegPopenEncoder()
{
	if (ffmpeg)
	{
		fflush(ffmpeg);
		if (pclose(ffmpeg) == -1)
			perror("FfmpegPopenEncoder: pclose");
		ffmpeg = nullptr;
	}
}
void FfmpegPopenEncoder::send_frame(const sf::Texture &txr)
{
	glBindTexture(GL_TEXTURE_2D, txr.getNativeHandle());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, video_size.x, video_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, txr.getNativeHandle(), 0);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[current_frame % NUM_PBOS]);
	glReadPixels(0, 0, video_size.x, video_size.y, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// Map previous PBO and write to FFmpeg
	int prev_idx = (current_frame + 1) % NUM_PBOS;
	if (current_frame >= NUM_PBOS - 1) { // Only start reading after we've filled the queue
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[prev_idx]);
		auto ptr = static_cast<std::uint8_t*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
		if (ptr) {
			fwrite(ptr, 1, video_size.x * video_size.y * 4, ffmpeg);
		}
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFlush();
	current_frame++;

	// send_frame(txr.copyToImage());
}

void FfmpegPopenEncoder::send_frame(const sf::Image &img)
{
	const auto pixels = img.getPixelsPtr();
	std::size_t size = img.getSize().x * img.getSize().y * 4;
	if (fwrite(pixels, 1, size, ffmpeg) != size)
		throw std::runtime_error("Failed to write frame to ffmpeg stdin");
}


} // namespace audioviz::media
