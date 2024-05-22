#include "viz/StereoSpectrum.hpp"

#include <iostream>

namespace viz
{

StereoSpectrum::StereoSpectrum(int sample_size)
	: _left(sample_size),
	  _right(sample_size)
{
	_left.set_backwards(true);
}

void StereoSpectrum::set_bar_spacing(int spacing)
{
	assert(_left.bar.get_spacing() == _right.bar.get_spacing());
	if (_left.bar.get_spacing() == spacing)
		return;
	_left.set_bar_spacing(spacing);
	_right.set_bar_spacing(spacing);
	update_spectrums();
}

void StereoSpectrum::set_rect(const sf::IntRect &rect)
{
	if (this->rect == rect)
		return;
	this->rect = rect;
	update_spectrums();
}

void StereoSpectrum::update_spectrums()
{
	assert(_left.bar.get_spacing() == _right.bar.get_spacing());

	const auto half_width = rect.width / 2.f;
	const auto half_bar_spacing = _left.bar.get_spacing() / 2.f;

	const sf::IntRect
		left_half{
			rect.getPosition(),
			{half_width - half_bar_spacing, rect.height}},
		right_half{
			{rect.left + half_width + half_bar_spacing, rect.top},
			{half_width - half_bar_spacing, rect.height}};

	const auto dist_between_rects = right_half.left - (left_half.left + left_half.width);
	assert(dist_between_rects == _left.bar.get_spacing());

	_left.set_rect(left_half);
	_right.set_rect(right_half);

	assert(_left.data().size() == _right.data().size());
}

void StereoSpectrum::do_fft(const float *const stereo_audio)
{
	_left.do_fft(stereo_audio, 2, 0, true);
	_right.do_fft(stereo_audio, 2, 1, true);
}

void StereoSpectrum::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	target.draw(_left, states);
	target.draw(_right, states);
}

} // namespace viz
