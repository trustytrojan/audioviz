---@diagnostic disable: lowercase-global, undefined-global

size = { 1280, 720 }
print('n9')
fa = tt.FrequencyAnalyzer.new(3000)
print('n91')
cs = viz.ColorSettings.new()
print('n92')
ps = viz.ParticleSystem.new(50)
print('n93')
ss = viz.StereoSpectrum.new(cs)
print('n94')
sc = viz.ScopeDrawable.new(cs)

-- cs.wheel.rate = 0.005 -- this is broken

print('n0')
viz = audioviz.new(size, args.media_url, fa, cs, ss, ps, 4)
print('n1')
viz:add_default_effects()
print('n2')
-- viz:set_text_font('/usr/share/fonts/TTF/Iosevka-Regular.ttc')
viz:set_text_font(os.getenv('LocalAppData') .. '\\Microsoft\\Windows\\Fonts\\Iosevka-Regular.ttc')

start_in_window(viz)
