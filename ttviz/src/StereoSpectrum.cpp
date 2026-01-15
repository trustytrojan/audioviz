#include "StereoSpectrum.hpp"

StereoSpectrum::StereoSpectrum(const sf::IntRect &rect, const audioviz::ColorSettings &color)
	: rect{rect},
	  _left{color},
	  _right{color}
{
	update_spectrum_rects();
}

void StereoSpectrum::update_spectrum_rects()
{
	assert(_left.get_bar_spacing() == _right.get_bar_spacing());

	const auto half_width{rect.size.x / 2.f};
	const auto half_bar_spacing{_left.get_bar_spacing() / 2.f};

	const sf::IntRect left_half{rect.position, {half_width - half_bar_spacing, rect.size.y}},
		right_half{
			{rect.position.x + half_width + half_bar_spacing, rect.position.y},
			{half_width - half_bar_spacing, rect.size.y}};

	const auto dist_between_rects{right_half.position.x - (left_half.position.x + left_half.size.x)};
	assert(dist_between_rects == _left.get_bar_spacing());

	_left.set_rect(left_half);
	_right.set_rect(right_half);

	assert(_left.get_bar_count() == _right.get_bar_count());
}

void StereoSpectrum::set_bar_width(int width)
{
	_left.set_bar_width(width);
	_right.set_bar_width(width);
	update_spectrum_rects();
}

void StereoSpectrum::set_bar_spacing(int spacing)
{
	_left.set_bar_spacing(spacing);
	_right.set_bar_spacing(spacing);
	update_spectrum_rects();
}

void StereoSpectrum::set_rect(const sf::IntRect &rect)
{
	if (this->rect == rect)
		return;
	this->rect = rect;
	update_spectrum_rects();
}

void StereoSpectrum::update(
	audioviz::FrequencyAnalyzer &fa, audioviz::StereoAnalyzer &sa, audioviz::BinPacker &bp, audioviz::Interpolator &ip)
{
	assert(_left.get_bar_count() == _right.get_bar_count());
	_left.update(fa, sa.left(), bp, ip);
	_right.update(fa, sa.right(), bp, ip);
}

void StereoSpectrum::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	target.draw(_left, states);
	target.draw(_right, states);
}
