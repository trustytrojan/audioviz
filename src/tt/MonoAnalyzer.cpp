#include "tt/MonoAnalyzer.hpp"

namespace tt
{

void MonoAnalyzer::resize(const int size)
{
	_spectrum.resize(size);
}

void MonoAnalyzer::analyze(tt::FrequencyAnalyzer &fa, const float *const audio)
{
	fa.copy_to_input(audio);
	fa.render(_spectrum);
}

} // namespace tt
