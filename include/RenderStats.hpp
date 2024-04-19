#pragma once

#include <SFML/System.hpp>

struct Statistic
{
	int64_t current, total, avg, min, max, count;
	const char *const name;

public:
	static void printHeader(FILE *const stream)
	{
		fprintf(stream, "%-12s%-12s%-12s%-12s%-12s%-12s\n", "Name", "Curr", "Avg", "Total", "Min", "Max");
	}

	Statistic(const char *const name)
		: name(name) {}

	void update(int64_t current)
	{
		total += current;
		++count;
		avg = total / count;
		min = std::min(min, current);
		max = std::max(max, current);
		this->current = current;
	}

	void printStats(FILE *const stream, bool newline = false) const
	{
		fprintf(stream, "%-12s%-12ld%-12ld%-12ld%-12ld%-12ld", name, current, avg, total, min, max);
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