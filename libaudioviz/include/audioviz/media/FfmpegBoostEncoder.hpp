#include <SFML/Graphics.hpp>
#include <audioviz/Base.hpp>
#include <audioviz/media/FfmpegEncoder.hpp>
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/search_path.hpp>

namespace bp = boost::process::v1;

namespace audioviz::media
{

class FfmpegBoostEncoder : public FfmpegEncoder
{
private:
	bp::child c;
	// can't use basic_pipe<non-char type> because LLVM deprecated std::char_traits<non-char type>
	bp::pipe video_in;

public:
	FfmpegBoostEncoder(
		const audioviz::Base &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec);
	~FfmpegBoostEncoder();

	void send_frame(const sf::Image &img);
	inline void send_frame(const sf::Texture &txr) { send_frame(txr.copyToImage()); }
};

} // namespace audioviz::media
