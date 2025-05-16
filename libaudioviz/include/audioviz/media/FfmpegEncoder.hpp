#include <SFML/Graphics.hpp>
#include <audioviz/Base.hpp>
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/search_path.hpp>

namespace bp = boost::process::v1;

namespace audioviz::media
{

class FfmpegEncoder
{
public:
#ifdef LINUX
	static std::string detect_vaapi_device();
#endif

private:
	bp::child c;
	// can't use basic_pipe<non-char type> because LLVM deprecated std::char_traits<non-char type>
	bp::pipe video_in;

public:
	FfmpegEncoder(
		const audioviz::Base &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec);
	~FfmpegEncoder();

	void send_frame(const sf::Image &img);
	inline void send_frame(const sf::Texture &txr) { send_frame(txr.copyToImage()); }
};

} // namespace audioviz::media
