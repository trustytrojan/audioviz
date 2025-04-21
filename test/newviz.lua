---@diagnostic disable: lowercase-global, undefined-global
package.cpath = package.cpath .. ';build/luaviz/?.so'
luaviz = require('luaviz')

if not arg[1] then
	print('media file required')
	os.exit(1)
end

SIZE = { 1280, 720 }
SPECTRUM_SIZE = { 500, 500 }
FFT_SIZE = 3000
fa = luaviz.FrequencyAnalyzer.new(FFT_SIZE)
sa = luaviz.StereoAnalyzer.new()
cs = luaviz.ColorSettings.new()
lbsd = luaviz.BarSpectrumDrawable.new({ { 0, 0 }, SPECTRUM_SIZE }, cs)
rbsd = luaviz.BarSpectrumDrawable.new({ { 500, 0 }, SPECTRUM_SIZE }, cs)
media = luaviz.FfmpegCliBoostMedia.new(arg[1], {})
viz = luaviz.Base.new(SIZE, media)
viz:set_audio_frames_needed(FFT_SIZE)

-- assert that either lbsd or rbsd can be used to configure the analyzer
-- aka set the analyzer's buffer size to have enough data for all the spectrum bars
assert(lbsd:bar_count() == rbsd:bar_count(), 'bar counts are not the same!')

lbsd:set_debug_rect(true)
rbsd:set_debug_rect(true)

lstates = luaviz.sfRenderStates.new()
-- lstates.transform:rotateDegrees(90)

rstates = luaviz.sfRenderStates.new()
-- rstates.transform:rotateDegrees(-90)

spectrum_layer = viz:add_layer('spectrum', 0)
spectrum_layer:set_orig_cb(function(orig_rt)
	orig_rt:clear({ 0, 0, 0, 0 })
	lbsd:configure_analyzer(sa)
	-- we asserted earlier that either lbsd or rbsd can be used to configure the analyzer
	viz:perform_fft(fa, sa)
	lbsd:update(sa:left_data())
	rbsd:update(sa:right_data())
	orig_rt:draw(lbsd, lstates)
	orig_rt:draw(rbsd, rstates)
	orig_rt:display()
end)

viz:start_in_window(arg[0])
