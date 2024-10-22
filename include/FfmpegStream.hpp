#pragma once

#include <boost/json.hpp>

namespace bj = boost::json;

class FfmpegStream
{
	const bj::object obj;

public:
	FfmpegStream(const bj::object &obj)
		: obj{obj}
	{
	}

	int index() const { return obj.at("index").as_int64(); }
	std::string_view codec_name() const { return obj.at("codec_name").as_string(); }
	std::string_view codec_type() const { return obj.at("codec_type").as_string(); }
	std::string_view sample_fmt() const { return obj.at("sample_fmt").as_string(); }
	int sample_rate() const { return obj.at("sample_rate").as_int64(); }
	int num_channels() const { return obj.at("channels").as_int64(); }
};
