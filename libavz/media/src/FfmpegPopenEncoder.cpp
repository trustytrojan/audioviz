#include "util.hpp"
#include <GL/glew.h>
#include <avz/media/FfmpegPopenEncoder.hpp>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace avz
{

FfmpegPopenEncoder::FfmpegPopenEncoder(
	const std::string &media_url,
	const unsigned video_width,
	const unsigned video_height,
	const int framerate,
	const std::string &outfile,
	const std::string &vcodec,
	const std::string &acodec)
	: video_width{video_width},
	  video_height{video_height}
{
	glewInit();
	glGenBuffers(NUM_PBOS, pbos);

	for (unsigned int pbo : pbos)
	{
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
		glBufferData(GL_PIXEL_PACK_BUFFER, byte_size, nullptr, GL_STREAM_READ);
	}
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	std::ostringstream cmd_stream;
	cmd_stream << "ffmpeg -hide_banner -hwaccel auto -y ";

	// input 0: raw video stream from avz
	cmd_stream << "-f rawvideo -pix_fmt rgba "
			   << "-s " << video_width << "x" << video_height << " "
			   << "-r " << framerate << " "
			   << "-i - ";

	// input 1: media used in avz
	cmd_stream << "-ss -0.1 ";
	if (media_url.find("http") != std::string::npos)
		cmd_stream << "-reconnect 1 ";
	cmd_stream << "-i \"" << media_url << "\" ";

#ifdef __linux__
	// if on linux and vaapi encoder used, detect a vaapi device for usage
	if (vcodec.find("vaapi") != std::string::npos)
	{
		if (const auto vaapi_device = util::detect_vaapi_device(); !vaapi_device.empty())
			cmd_stream << "-vaapi_device " << vaapi_device << " -vf vflip,format=nv12,hwupload ";
		else
			std::cerr << "[FfmpegPopenEncoder] failed to find a vaapi device for h264_vaapi ffmpeg encoder!\n";
	}
#else
	// vertically flip because pixels from opengl functions are bottom-up rows
	cmd_stream << "-vf vflip ";
#endif

	// stream mapping
	cmd_stream << "-map 0 -map 1:a ";

	// encoders
	cmd_stream << "-c:v " << vcodec << " -c:a " << acodec << " ";

	// end on shortest input stream
	cmd_stream << "-shortest " << outfile;

	const auto command = cmd_stream.str();
	std::cout << "[FfmpegPopenEncoder] command: " << command << '\n';
	if (!(ffmpeg = util::popen_utf8(command, POPEN_W_MODE)))
		throw std::runtime_error{
			"[FfmpegPopenEncoder] Failed to start ffmpeg process with popen: " + std::string{strerror(errno)}};
}

FfmpegPopenEncoder::~FfmpegPopenEncoder()
{
	if (!ffmpeg)
		return;
	if (fflush(ffmpeg) == EOF)
		perror("[~FfmpegPopenEncoder] fflush");
	if (pclose(ffmpeg) == -1)
		perror("[~FfmpegPopenEncoder] pclose");
}

void FfmpegPopenEncoder::send_frame(const unsigned glTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTexture, 0);

	GLint previous_pack_alignment = 0;
	glGetIntegerv(GL_PACK_ALIGNMENT, &previous_pack_alignment);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[current_frame % NUM_PBOS]);
	glReadPixels(0, 0, video_width, video_height, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// Map previous PBO and write to FFmpeg
	const auto prev_idx = (current_frame + 1) % NUM_PBOS;

	if (current_frame >= NUM_PBOS - 1)
	{
		// Only start reading after we've filled the queue
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[prev_idx]);
		const auto *const ptr = static_cast<std::byte *>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
		if (ptr && fwrite(ptr, 1, byte_size, ffmpeg) < byte_size)
			throw std::runtime_error{"[FfmpegPopenEncoder::send_frame] fwrite returned < size!"};
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, previous_pack_alignment);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glFlush();

	++current_frame;
}

} // namespace avz
