#pragma once

#include <audioviz/ScopeDrawable.hpp>

namespace audioviz
{

template <typename ShapeType>
class StereoScope : public sf::Drawable
{
	ScopeDrawable<ShapeType> _left, _right;
	sf::IntRect rect;

public:
	StereoScope(const ColorSettings &left_color, const ColorSettings &right_color)
		: _left{left_color},
		  _right{right_color}
	{
	}

	StereoScope(const sf::IntRect &rect, const ColorSettings &left_color, const ColorSettings &right_color)
		: _left{left_color},
		  _right{right_color},
		  rect{rect}
	{
		update_scope_rects();
	}

	void set_left_backwards(const bool b) { _left.set_backwards(b); }
	void set_right_backwards(const bool b) { _right.set_backwards(b); }

	void set_shape_spacing(int spacing)
	{
		_left.set_shape_spacing(spacing);
		_right.set_shape_spacing(spacing);
	}

	void set_shape_width(int width)
	{
		_left.set_shape_width(width);
		_right.set_shape_width(width);
	}

	void set_fill_in(const bool b)
	{
		_left.set_fill_in(b);
		_right.set_fill_in(b);
	}

	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;
		update_scope_rects();
	}

	size_t get_shape_count() const
	{
		assert(_left.get_shape_count() == _right.get_shape_count());
		return _left.get_shape_count();
	}

	void update(const std::span<float> &left, const std::span<float> &right)
	{
		_left.update(left);
		_right.update(right);
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states = {}) const override
	{
		target.draw(_left, states);
		target.draw(_right, states);
	}

private:
	void update_scope_rects()
	{
		_left.set_rect(rect);
		_right.set_rect(rect);
	}
};

} // namespace audioviz