#pragma once

#include <cstdint>

namespace pa
{
	using Float32 = float;
	using Int32 = int32_t;
	// clang-format off
	struct Int24 { uint8_t bytes[3]; };
	// clang-format on
	using Int16 = int16_t;
	using Int8 = int8_t;
	using UInt8 = uint8_t;
}
