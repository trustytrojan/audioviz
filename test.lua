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

vcodec = 'h264_qsv'

function benchmark_encode(encode_func, file)
	local time = measure_runtime(function() encode_func(viz, file, vcodec, 'copy') end)
	print(file .. ' = ' .. time)
end

benchmark_encode(encode_without_window, 'out-mt.mp4')
benchmark_encode(encode_without_window, 'out-st.mp4')
