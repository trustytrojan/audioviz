#include <GL/glew.h>
#include <audioviz/media/FfmpegPopenEncoder.hpp>
#include <audioviz/util.hpp>
#include <iostream>
#include <stdexcept>

namespace audioviz
{

FfmpegPopenEncoder::FfmpegPopenEncoder(
	const audioviz::Base &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
	: video_size{viz.size}
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
			cmd_stream << "-vaapi_device " << vaapi_device << " -vf vflip,format=nv12,hwupload ";
		else
			std::cerr << "failed to find a vaapi device for h264_vaapi ffmpeg encoder!\n";
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
	std::cout << "FfmpegPopenEncoder: command: " << command << '\n';
	if (!(ffmpeg = util::popen_utf8(command, POPEN_W_MODE)))
		throw std::runtime_error{"Failed to start ffmpeg process with popen" + std::string{strerror(errno)}};
}

FfmpegPopenEncoder::~FfmpegPopenEncoder()
{
	if (ffmpeg)
	{
		fflush(ffmpeg);
		if (pclose(ffmpeg) == -1)
			perror("FfmpegPopenEncoder: pclose");
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
	const auto prev_idx = (current_frame + 1) % NUM_PBOS;

	if (current_frame >= NUM_PBOS - 1)
	{
		// Only start reading after we've filled the queue
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[prev_idx]);
		const auto *const ptr = static_cast<std::byte *>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
		if (ptr && fwrite(ptr, 1, byte_size, ffmpeg) < byte_size)
			throw std::runtime_error{"FfmpegPopenEncoder: fwrite returned < size!"};
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFlush();

	++current_frame;
}

void FfmpegPopenEncoder::send_frame(const sf::Image &img)
{
	assert(img.getSize() == video_size && "passed image size != configured video size!");
	if (fwrite(img.getPixelsPtr(), 1, byte_size, ffmpeg) < byte_size)
		throw std::runtime_error{"FfmpegPopenEncoder: fwrite returned < size!"};
}

} // namespace audioviz
