---@diagnostic disable: lowercase-global, undefined-global

size = { 1280, 720 }
fa = tt.FrequencyAnalyzer.new(3000)
cs = viz.ColorSettings.new()
ps = viz.ParticleSystem.new({ {}, size }, 50)
ss = viz.StereoSpectrum.new({ {}, size }, cs)
sc = viz.ScopeDrawable.new({ {}, size }, cs)

cs:set_wheel_rate(0.005)

_viz = audioviz.new(size, args.media_url, fa, cs, ss, ps, 4)
_viz:add_default_effects()

if LINUX then
	_viz:set_text_font('/usr/share/fonts/TTF/Iosevka-Regular.ttc')
	vcodec = 'h264_vaapi'
elseif WIN32 then
	_viz:set_text_font(os.getenv('LocalAppData') .. '\\Microsoft\\Windows\\Fonts\\Iosevka-Regular.ttc')
	vcodec = 'h264_qsv'
end

test_layer = _viz:add_layer('test', 4)
test_layer:set_orig_cb(function(orig_rt)
	orig_rt:clear({50, 0, 0, 125})
	orig_rt:display()
end)
-- test_layer:set_fx_cb(function(orig_rt, fx_rt, target)
-- 	target:draw(fx_rt:sprite())
-- end)
-- test_layer:set_fx_cb(viz.Layer.DRAW_FX_RT)

-- viz:remove_layer('test')

start_in_window(_viz)
-- encode(viz, 'out.mp4', vcodec, 'copy')
