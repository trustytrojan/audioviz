#pragma once

#include <SFML/Graphics.hpp>
#include <avz/gfx/ColorSettings.hpp>
#include <avz/gfx/util.hpp>
#include <span>
#include <vector>

namespace avz
{

class ScopeDrawable : public sf::Drawable
{
	sf::IntRect rect{};
	bool backwards{false};
	struct
	{
		int width{10}, spacing{5}, count{};
	} shape;
	bool fill_in{false};
	float audio_duration{0.f};
	int sample_rate{0};
	const ColorSettings &color;
	sf::VertexArray vertex_array;
	std::vector<float> m_window;
	std::vector<float> m_resampled;

public:
	ScopeDrawable(const ColorSettings &color, const bool backwards = false);
	ScopeDrawable(const sf::IntRect &rect, const ColorSettings &color, const bool backwards = false);

	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;
		update_shapes();
	}

	void set_shape_spacing(int spacing)
	{
		if (spacing == shape.spacing)
			return;
		shape.spacing = spacing;
		update_shapes();
	}

	void set_shape_width(int width)
	{
		if (width == shape.width)
			return;
		shape.width = width;
		update_shapes();
	}

	void set_fill_in(const bool b)
	{
		if (fill_in == b)
			return;
		fill_in = b;
	}

	void set_backwards(const bool b)
	{
		if (backwards == b)
			return;
		backwards = b;
		update_shapes();
	}

	void set_audio_duration(const float seconds) { audio_duration = seconds; }
	void set_sample_rate(const int rate) { sample_rate = rate; }

	int get_shape_count() const { return shape.count; }
	int get_shape_spacing() const { return shape.spacing; }
	int get_shape_width() const { return shape.width; }
	float get_audio_duration() const { return audio_duration; }
	int get_sample_rate() const { return sample_rate; }

	void update(std::span<const float> audio);
	void update(std::span<const float> audio, int sample_rate);

	void draw(sf::RenderTarget &target, sf::RenderStates states = {}) const override;

private:
	void update_shapes();
	void update_shape_colors();
	int get_shape_vertex_index(int shape_idx, int vertex_num) const;
	int required_sample_count() const;
	void update_vertices(std::span<const float> samples);
};

} // namespace avz
