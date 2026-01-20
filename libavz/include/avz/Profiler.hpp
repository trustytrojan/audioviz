#pragma once

#include <SFML/System.hpp>
#include <algorithm>
#include <format>
#include <limits>
#include <string>
#include <vector>


namespace avz
{

class Profiler
{
	struct TimingStat
	{
		std::string name;
		float min{std::numeric_limits<float>::max()}, max{}, total{}, current{};
		size_t count{};
		inline float avg() const { return (count == 0) ? 0.0f : total / count; }
		inline operator std::string() const
		{
			return std::format("{:<20}{:<7.3f}{:<7.3f}{:<7.3f}{:<7.3f}\n", name, current, avg(), min, max);
		}
	};

	std::vector<TimingStat> stats;
	sf::Clock clock;
	std::string current_section;

public:
	inline void startSection(std::string_view name)
	{
		clock.restart();
		current_section = name;
	}

	inline void endSection()
	{
		auto &stat = get_or_create_timing_stat(current_section);
		const float time_ms = clock.getElapsedTime().asMicroseconds() / 1e3f;
		stat.current = time_ms;
		if (time_ms < stat.min)
			stat.min = time_ms;
		if (time_ms > stat.max)
			stat.max = time_ms;
		stat.total += time_ms;
		stat.count++;
	}

	inline void endStartSection(std::string_view name)
	{
		endSection();
		startSection(name);
	}

	inline std::string getSummary()
	{
		auto s = std::format("{:<20}{:<7}{:<7}{:<7}{:<7}\n", "", "curr", "avg", "min", "max");
		for (const auto &stat : stats)
			s += stat;
		return s;
	}

private:
	TimingStat &get_or_create_timing_stat(const std::string &label)
	{
		auto it = std::ranges::find_if(stats, [&label](auto &stat) { return stat.name == label; });
		if (it != stats.end())
			return *it;
		return stats.emplace_back(label);
	}
};

} // namespace avz
