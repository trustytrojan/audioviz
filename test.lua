---@diagnostic disable: lowercase-global, undefined-global

size = { 1280, 720 }
fa = tt.FrequencyAnalyzer.new(3000)
ps = viz.ParticleSystem.new({ {}, size }, 50)
ss = viz.StereoSpectrum.new()

viz = audioviz.new(size, args.media_url, fa, ss, ps, 4)
viz:add_default_effects()
viz:set_text_font('/usr/share/fonts/TTF/Iosevka-Regular.ttc')

ss:set_bar_width(1)
ss:set_bar_spacing(1)

start_in_window(viz)
