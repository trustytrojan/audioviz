#pragma once

#include <boost/process.hpp>
#include <boost/json.hpp>

namespace bp = boost::process;
namespace bj = boost::json;

class Media2
{
	bp::child c;
	bp::ipstream audio;
	std::optional<bp::ipstream> video;

public:
	Media2(const std::string &url)
	{

	}
};
