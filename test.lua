---@diagnostic disable: lowercase-global, undefined-global

SIZE = { 1280, 720 }
fa = tt.FrequencyAnalyzer.new(3000)
ps = viz.ParticleSystem.new({ {}, SIZE }, 50)
ss = viz.StereoSpectrum.new()

viz = audioviz.new(SIZE, 'Music/After Rain [HvRyTV-RpvE].mp3', fa, ss, ps, 4)
viz:add_default_effects()
viz:set_text_font('/usr/share/fonts/TTF/Iosevka-Regular.ttc')

ss:set_bar_width(1)
ss:set_bar_spacing(1)
ss:set_left_backwards(false)
ss:set_right_backwards(true)

start_in_window(viz)
