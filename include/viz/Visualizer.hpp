#pragma once

#include <SFML/Graphics/Drawable.hpp>

namespace viz
{

/**
 * Interface for an `sf::Drawable` that visualizes audio to some capacity.
 */
template <typename SampleType>
class AudioVisualizer : public sf::Drawable
{
public:
	virtual void process(const SampleType *const audio) = 0;
};

} // namespace viz
