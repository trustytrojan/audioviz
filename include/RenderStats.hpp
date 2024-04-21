#pragma once

#include <SFML/System.hpp>
#include <climits>

struct Statistic
{
	uint64_t current, total = 0, min = INT64_MAX, max = 0, count = 0;
	const char *const name;

public:
	static void printHeader(FILE *const stream)
	{
		fprintf(stream, "%-12s%-12s%-12s%-12s%-12s\n", "Name", "Curr", "Avg", "Min", "Max");
	}

	Statistic(const char *const name)
		: name(name) {}

	void update(uint64_t current)
	{
		this->current = current;
		total += current;
		++count;
		min = std::min(min, current);
		max = std::max(max, current);
	}

	void printStats(FILE *const stream, bool newline = false) const
	{
		fprintf(stream, "%-12s%-12ld%-12ld%-12ld%-12ld", name, current, total / count, min, max);
		if (newline)
			putc('\n', stream);
	}
};

class RenderStats
{
	sf::Clock clock;
	FILE *stream;
	Statistic drawTime = "Draw (us)", fps = "FPS";

	class ScopedTimer
	{
		RenderStats &rs;

	public:
		ScopedTimer(RenderStats &rs) : rs(rs) { rs.restartClock(); }
		~ScopedTimer() { rs.updateAndPrint(); }
	};

public:
	RenderStats(FILE *const stream = stderr)
		: stream(stream){};

	void setOutputStream(FILE *const stream)
	{
		this->stream = stream;
	}

	void printHeader()
	{
		Statistic::printHeader(stream);
	}

	ScopedTimer createScopedTimer()
	{
		return ScopedTimer(*this);
	}

	void restartClock()
	{
		clock.restart();
	}

	void updateAndPrint()
	{
		updateStats();
		printStats();
	}

	void updateStats()
	{
		const auto time = clock.getElapsedTime();
		drawTime.update(time.asMicroseconds());
		fps.update(1 / time.asSeconds());
	}

	void printStats() const
	{
		static bool printedOnce = false;
		if (printedOnce)
			fprintf(stream, "\r\e[1A\e[2K");
		drawTime.printStats(stream, true);
		fps.printStats(stream);
		printedOnce = true;
	}
};