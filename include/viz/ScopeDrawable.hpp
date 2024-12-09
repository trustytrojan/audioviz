#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>


namespace viz
{

template <typename ShapeType>
class ScopeDrawable : public sf::Drawable
{
	sf::IntRect rect;
	bool backwards = false;
	struct
	{
		int width = 1, height = 1, spacing = 1;
	} shape;

	//fill in osc from middle screen to edges 
	bool fill_in = false;

	//height of oscilloscope
	double amplitude = 1;

	//shake_mag increases the amount of radians it shakes
	//shake_act increases sensitivity to start shaking
	double shake_mag = 0;
	double shake_act = .3;
	bool shake_rot = false;
	bool shake_pos = true;

	sf::Angle angle = sf::degrees(0);
	sf::Transformable tf;
	std::vector<ShapeType> shapes;

public:
	ScopeDrawable(const sf::IntRect &rect, const bool backwards = false)
		: rect{rect},
		  backwards{backwards}
	{
		update_shape_x_positions();

		const sf::Vector2f _origin{rect.size.x / 2.f, rect.size.y / 2.f};
		tf.setOrigin(_origin);
		tf.setPosition(_origin);
		tf.setRotation(angle);
	}

	void set_shape_spacing(const int spacing)
	{
		if (shape.spacing == spacing)
			return;
		shape.spacing = spacing;
		update_shape_x_positions();
	}

	void set_shape_width(const int width)
	{
		if (shape.width == width)
			return;
		shape.width = width;
		update_shape_x_positions();
	}
	void set_shape_height(const int height)
	{
		if (shape.height == height)
			return;
		shape.height = height;
		update_shape_x_positions();
	}

	void set_fill_in(bool _fill) { fill_in = _fill; }
 
	// set magnitude of rotations
	void set_shake_mag(double _shake) {shake_mag = _shake; }

	void set_amplitude(double _amp) {amplitude = _amp; }
	//allow position shake
	void set_shake_pos(bool _spos) {shake_pos = _spos;}

	//allow rotational shake
	void set_shake_rot(bool _srot) {shake_rot = _srot;}

	// set amount required to shake
	void set_shake_activation(double _act) {shake_act = _act; }

	// set the area in which the spectrum will be drawn to
	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;
		update_shape_x_positions();
	}

	void set_backwards(bool b) { backwards = b; }

	void set_rotation_angle(sf::Angle angle) { tf.setRotation(angle); }

	void set_center_point_polar(double radius, sf::Angle angle)
	{
		sf::Vector2f origin_{rect.size.x/2.f, rect.size.y/2.f};	
		sf::Vector2f coord{origin_.x + radius * cos(angle.asRadians()), origin_.y - radius * sin(angle.asRadians())};

		tf.setPosition({coord});
	}

	void set_center_point_cartesian(double _x, double _y)
	{
		sf::Vector2f origin_{rect.size.x/2.f, rect.size.y/2.f};	
		sf::Vector2f coord{origin_.x + _x, origin_.y - _y};

		tf.setPosition({coord});
	}

	size_t get_shape_count() const { return shapes.size(); }

	//shakes osc based on magnitude and sensitivity using audio at the middle of the vector
	void update_shake(const std::span<float> &audio)
	{
		if(shake_mag == 0)
			return;


		assert(audio.size() >= shapes.size());

		if(shake_rot)
		{
			if(audio[audio.size()/2] < shake_act || audio[audio.size()/2] > -shake_act)
			{
				set_rotation_angle(sf::Angle(shake_mag*sf::degrees(audio[audio.size()/2])));
			}
		}

		//shakes position vertically and horizontally
		if(shake_pos)
		{
			set_center_point_cartesian(shake_mag * audio[audio.size()/2], shake_mag * audio[audio.size()/2]);
		}
		
	}

	void update_shape_positions(const std::span<float> &audio)
	{
		assert(audio.size() >= shapes.size());

		for (int i = 0; i < (int)shapes.size(); ++i)
		{
			const auto half_height = rect.size.y / 2.f;
			const auto half_heightx = rect.size.x / 2.f;

			if (!fill_in)
			{
				shapes[i].setPosition({
					shapes[i].getPosition().x,
					std::clamp(half_height + (-half_height * audio[i]* (float)amplitude), 0.f, (float)rect.size.y),
				});
			}
			else
			{
				shapes[i].setPosition({
					shapes[i].getPosition().x,
					std::clamp(half_height, 0.f, (float)rect.size.y),
				});
				shapes[i].setSize({shape.width, (-half_height * audio[i] * (float)amplitude)});
			}
		}
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override
	{
		for (const auto &shape : shapes)
			target.draw(shape, tf.getTransform());
	}

private:
	void update_shape_x_positions()
	{
		const int shape_count = rect.size.x / (shape.width + shape.spacing);
		shapes.resize(shape_count);

		for (int i = 0; i < shapes.size(); ++i)
		{
			if constexpr (std::is_base_of_v<sf::CircleShape, ShapeType>)
				shapes[i].setRadius(shape.width);
			else if constexpr (std::is_base_of_v<sf::RectangleShape, ShapeType>)
				shapes[i].setSize({shape.width, shape.height});
			else
				shapes[i].setScale(shape.width);

			// clang-format off
			const auto x = backwards
				? rect.position.x + rect.size.x - shape.width - i * (shape.width + shape.spacing)
				: rect.position.x + i * (shape.width + shape.spacing);
			const auto y = backwards
				? rect.position.y + rect.size.y - shape.width - i * (shape.width + shape.spacing)
				: rect.position.y + i * (shape.width + shape.spacing);
			// clang-format on

			shapes[i].setPosition({x, rect.position.y + rect.size.y / 2});
		}
	}
};

} // namespace viz
