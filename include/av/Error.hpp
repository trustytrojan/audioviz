#pragma once

#include <stdexcept>

extern "C"
{
#include <libavutil/error.h>
}

namespace av
{
	class Error : public std::runtime_error
	{
		static std::string make_err_string(const int errnum)
		{
			char errbuf[AV_ERROR_MAX_STRING_SIZE];
			av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, errnum);
			return errbuf;
		}

	public:
		const char *const func;
		const int errnum;

		Error(const char *const func, const int errnum)
			: std::runtime_error(std::string(func) + ": " + make_err_string(errnum)),
			  func(func),
			  errnum(errnum) {}
	};
}
