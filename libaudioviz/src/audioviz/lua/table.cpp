#include <audioviz/ColorSettings.hpp>
#include <audioviz/ParticleSystem.hpp>
#include <audioviz/ScopeDrawable.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/StereoSpectrum.hpp>
#include <audioviz/VerticalBar.hpp>
#include <audioviz/Base.hpp>
#include <audioviz/fft/StereoAnalyzer.hpp>
#include <audioviz/lua/table.hpp>
#include <audioviz/media/FfmpegCliBoostMedia.hpp>
#include <audioviz/util.hpp>

sf::IntRect table_to_intrect(const sol::table &tb)
{
	const auto pos = tb[1].get<sol::table>(), size = tb[2].get<sol::table>();
	return {
		pos.empty() ? sf::Vector2i{} : sf::Vector2i{pos[1].get<int>(), pos[2].get<int>()},
		size.empty() ? sf::Vector2i{} : sf::Vector2i{size[1].get<int>(), size[2].get<int>()},
	};
}

sf::Vector2u table_to_vec2u(const sol::table &tb)
{
	if (tb.empty())
		return {};
	return {tb[1].get<uint>(), tb[2].get<uint>()};
}

sf::Color table_to_color(const sol::table &tb)
{
	auto color = sf::Color::Black;
	if (tb[1].valid())
		color.r = tb[1];
	if (tb[2].valid())
		color.g = tb[2];
	if (tb[3].valid())
		color.b = tb[3];
	if (tb[4].valid())
		color.a = tb[4];
	if (tb["r"].valid())
		color.r = tb["r"];
	if (tb["g"].valid())
		color.r = tb["g"];
	if (tb["b"].valid())
		color.b = tb["b"];
	if (tb["a"].valid())
		color.a = tb["a"];
	return color;
}

namespace audioviz::lua
{

table::table(const sol::table &t)
	: sol::table{t}
{
	using FA = fft::FrequencyAnalyzer;
	using AA = fft::AudioAnalyzer;
	using SA = fft::StereoAnalyzer;
	using RT = RenderTexture;
	using CS = ColorSettings;
	using CPS = ParticleSystem<Particle<sf::CircleShape>>;
	using BSD = SpectrumDrawable<VerticalBar>;
	using BSS = StereoSpectrum<VerticalBar>;
	using BSC = ScopeDrawable<VerticalBar>;

	// clang-format off

	new_usertype<FA>("FrequencyAnalyzer",
		"new", sol::constructors<FA(int)>(),
		"set_fft_size", &FA::set_fft_size,
		"set_interp_type", &FA::set_interp_type,
		"set_window_func", &FA::set_window_func,
		"set_accum_method", &FA::set_accum_method,
		"set_scale", &FA::set_scale,
		"set_nth_root", &FA::set_nth_root
	);

	new_usertype<AA>("AudioAnalyzer",
		"new", sol::constructors<AA(int)>(),
		"resize", &AA::resize
	);

	new_usertype<SA>("StereoAnalyzer",
		"new", sol::constructors<SA()>(),
		sol::base_classes, sol::bases<AA>()
	);

	new_usertype<RT>("RenderTexture",
		"new", sol::constructors<RT(sf::Vector2u, int)>(),
		"display", &RT::display,
		"sprite", &RT::sprite,
		"clear", [](RT &self, const sol::table &table)
		{
			self.clear(table_to_color(table));
		},
		sol::base_classes, sol::bases<sf::RenderTarget>()
	);

	new_usertype<Layer>("Layer",
		"", sol::no_constructor,
		"set_orig_cb", &Layer::set_orig_cb,
		"set_fx_cb", &Layer::set_fx_cb,
		"set_auto_fx", &Layer::set_auto_fx,
		"add_drawable", [](Layer &self, const sf::Drawable &drawable)
		{
			self.drawables.emplace_back(&drawable);
		},
		"add_effect", [](Layer &self, const fx::Effect &effect)
		{
			self.effects.emplace_back(&effect);
		},
		"DRAW_FX_RT", sol::var(Layer::DRAW_FX_RT)
	);

	new_usertype<CPS>("CircleParticleSystem",
		"new", sol::constructors<CPS(int)>(),
		"new", sol::factories([](const sol::table &rect, const int particle_count)
		{
			return std::make_shared<CPS>(table_to_intrect(rect), particle_count);
		}),
		"set_displacement_direction", &CPS::set_displacement_direction,
		"set_start_position", &CPS::set_start_side,
		"set_rect", &CPS::set_rect,
		"set_particle_count", &CPS::set_particle_count,
		sol::base_classes, sol::bases<sf::Drawable>()
	);

	new_usertype<CS>("ColorSettings",
		"new", sol::constructors<CS>(),
		"set_mode", &CS::set_mode,
		"set_wheel_hsv", &CS::set_wheel_hsv,
		"set_wheel_ranges_start_hsv", &CS::set_wheel_ranges_start_hsv,
		"set_wheel_ranges_end_hsv", &CS::set_wheel_ranges_end_hsv,
		"set_wheel_rate", &CS::set_wheel_rate,
		"increment_wheel_time", &CS::increment_wheel_time
	);

	new_usertype<BSD>("BarSpectrumDrawable",
		"new", sol::constructors<BSD(CS&)>(),
		"new", sol::factories([](const sol::table &rect, CS &cs)
		{
			return std::make_shared<BSD>(table_to_intrect(rect), cs);
		}),
		"set_multiplier", &BSD::set_multiplier,
		"set_rect", &BSD::set_rect,
		"set_bar_width", &BSD::set_bar_width,
		"set_bar_spacing", &BSD::set_bar_spacing,
		"set_backwards", &BSD::set_backwards,
		sol::base_classes, sol::bases<sf::Drawable>()
	);

	new_usertype<BSS>("BarStereoSpectrum",
		"new", sol::constructors<BSS(CS&)>(),
		"new", sol::factories([](const sol::table &rect, CS &cs)
		{
			return std::make_shared<BSS>(table_to_intrect(rect), cs);
		}),
		"set_multiplier", &BSS::set_multiplier,
		"set_rect", [](BSS &self, const sol::table &rect)
		{
			self.set_rect(table_to_intrect(rect));
		},
		"set_bar_width", &BSS::set_bar_width,
		"set_bar_spacing", &BSS::set_bar_spacing,
		"set_left_backwards", &BSS::set_left_backwards,
		"set_right_backwards", &BSS::set_right_backwards,
		"update", &BSS::update,
		"get_bar_count", &BSS::get_bar_count,
		"configure_analyzer", &BSS::configure_analyzer,
		sol::base_classes, sol::bases<sf::Drawable>()
	);

	new_usertype<BSC>("ScopeDrawable",
		"new", sol::constructors<BSC(CS&)>(),
		"new", sol::factories([](const sol::table &rect, CS &cs)
		{
			return std::make_shared<BSC>(table_to_intrect(rect), cs);
		}),
		"set_rect", &BSC::set_rect,
		"set_shape_width", &BSC::set_shape_width,
		"set_shape_spacing", &BSC::set_shape_spacing,
		"set_fill_in", &BSC::set_fill_in,
		"set_backwards", &BSC::set_backwards,
		"set_rotation_angle", &BSC::set_rotation_angle,
		"set_center_point", &BSC::set_center_point,
		sol::base_classes, sol::bases<sf::Drawable>()
	);

	new_usertype<media::Media>("Media",
		"", sol::no_constructor
	);

	new_usertype<media::FfmpegCliBoostMedia>("FfmpegCliBoostMedia",
		"new", sol::factories([](const std::string &url, const sol::table &size)
		{
			return std::make_shared<media::FfmpegCliBoostMedia>(url, table_to_vec2u(size));
		}),
		sol::base_classes, sol::bases<media::Media>()
	);

	new_usertype<Base>("Base",
		"new", sol::factories([](const sol::table &rect, media::Media *const media)
		{
			return std::make_shared<Base>(table_to_vec2u(rect), media);
		}),
		"add_layer", &Base::add_layer,
		"get_layer", &Base::get_layer,
		"remove_layer", &Base::remove_layer,
		"perform_fft", &Base::perform_fft,
		"set_text_font", &Base::set_text_font,
		"set_audio_frames_needed", &Base::set_audio_frames_needed,
		sol::base_classes, sol::bases<sf::Drawable>()
	);

	set_function("start_in_window", &util::start_in_window);

	// clang-format on
}

} // namespace audioviz::lua
