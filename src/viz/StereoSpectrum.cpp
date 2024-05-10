#include "viz/StereoSpectrum.hpp"

namespace viz {

StereoSpectrum::StereoSpectrum(int sample_size)
	: sample_size(sample_size) {}

void StereoSpectrum::set_target_rect(const sf::IntRect &rect)
{
	const auto half_width = rect.width / 2.f;
	const auto half_bar_spacing = sd_left.bar.get_spacing() / 2.f;

	const sf::IntRect
		left_half{
			rect.getPosition(),
			{half_width - half_bar_spacing, rect.height}},
		right_half{
			{rect.left + half_width + half_bar_spacing, rect.top},
			{half_width - half_bar_spacing, rect.height}};

	const auto dist_between_rects = right_half.left - (left_half.left + left_half.width);
	assert(dist_between_rects == sd_left.bar.get_spacing());

	sd_left.set_target_rect(left_half, true);
	sd_right.set_target_rect(right_half);

	assert(sd_left.data().size() == sd_right.data().size());
}

void StereoSpectrum::do_fft(const float *const stereo_audio)
{
	sd_left.do_fft(stereo_audio, 2, 0, true);
	sd_right.do_fft(stereo_audio, 2, 1, true);
}

void StereoSpectrum::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	target.draw(sd_left, states);
	target.draw(sd_right, states);
}

} // namespace viz
