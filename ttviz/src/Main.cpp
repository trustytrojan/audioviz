#include "Main.hpp"
#include "imgui-SFML.h"
#include "imgui.h"

Main::Main(const int argc, const char *const *const argv)
	: args{argc, argv}
{
	const auto &size_args = args.get<std::vector<uint>>("--size");
	const sf::Vector2u size{size_args[0], size_args[1]};
	ttviz viz{size, args.get("media_url"), fa, cs, ss, ps};
	ps.set_rect({{}, (sf::Vector2i)size});
	use_args(viz);

	// --encode: render to video file
	switch (const auto &encode_args = args.get<std::vector<std::string>>("--encode"); encode_args.size())
	{
	case 0:
		// default behavior: render to window
		start_in_window(viz);
		break;
	case 1:
		viz.encode(encode_args[0]);
		break;
	case 2:
		viz.encode(encode_args[0], encode_args[1]);
		break;
	case 3:
		viz.encode(encode_args[0], encode_args[1], encode_args[2]);
		break;
	default:
		throw std::logic_error("--encode requires 1-3 arguments");
	}
}

static const char *layer_name_getter(void *user_data, int idx)
{
	return (*(const std::vector<audioviz::Layer> *)user_data)[idx].get_name().c_str();
}

void Main::start_in_window(audioviz::Base &viz)
{
#ifdef AUDIOVIZ_PORTAUDIO
	viz.set_audio_playback_enabled(true);
#endif

	sf::RenderWindow window{
		sf::VideoMode{viz.size},
		"ttviz",
		sf::Style::Titlebar,
		sf::State::Windowed,
		{.antiAliasingLevel = 4},
	};
	window.setVerticalSyncEnabled(!no_vsync);
	if (!ImGui::SFML::Init(window))
		throw std::runtime_error("ImGui::SFML::Init() failed");
	sf::Clock delta_clock;
	int bar_width = 10, bar_spacing = 5, fft_size = fa.get_fft_size();
	int selected_layer_idx = -1;
	while (window.isOpen() && viz.next_frame())
	{
		while (const auto event = window.pollEvent())
		{
			ImGui::SFML::ProcessEvent(window, *event);
			if (event->is<sf::Event::Closed>())
				window.close();
		}
		ImGui::SFML::Update(window, delta_clock.restart());

		ImGui::Begin("ttviz config");
		ImGui::SliderInt("bar width", &bar_width, 1, 100);
		ImGui::SliderInt("bar spacing", &bar_spacing, 0, 100);
		ImGui::SliderInt("fft size", &fft_size, 100, 10'000);
		ImGui::ListBox("layers", &selected_layer_idx, layer_name_getter, &viz.layers, viz.layers.size());
		ImGui::End();

		ss.set_bar_width(bar_width);
		ss.set_bar_spacing(bar_spacing);
		fa.set_fft_size(fft_size);
		if (selected_layer_idx > -1)
		{
			viz.remove_layer(viz.layers[selected_layer_idx].get_name());
			selected_layer_idx = -1;
		}

		window.clear();
		window.draw(viz);
		ImGui::SFML::Render(window);
		window.display();
	}
	ImGui::SFML::Shutdown();
}
