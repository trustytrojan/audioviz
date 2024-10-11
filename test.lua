---@diagnostic disable: lowercase-global, undefined-global

---@param callback function The function to be timed.
function measure_runtime(callback)
	local start_time = os.clock()
	callback()
	return os.clock() - start_time
end

SIZE = { 1280, 720 }
fa = tt.FrequencyAnalyzer.new(3000)
ps = viz.ParticleSystem.new({ {}, SIZE }, 50)
ss = viz.StereoSpectrum.new()

viz = audioviz.new(SIZE, args.media_url, fa, ss, ps, 4)
viz:add_default_effects()
-- viz:set_text_font('/usr/share/fonts/TTF/Iosevka-Regular.ttc')

ss:set_bar_width(1)
ss:set_bar_spacing(1)
-- ss:set_left_backwards(false)
-- ss:set_right_backwards(true)

start_in_window(viz)
-- st_time = measure_runtime(function() encode_without_window(viz, 'out.mp4', 'h264_vaapi', 'copy') end)
-- mt_time = measure_runtime(function() encode_without_window_mt(viz, 'out.mp4', 'h264_vaapi', 'copy') end)

-- print('st_time = ' .. st_time)
-- print('mt_time = ' .. mt_time)
