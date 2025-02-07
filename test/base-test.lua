---@diagnostic disable: lowercase-global, undefined-global
package.cpath = package.cpath .. ';/home/t/Projects/audioviz/build/libaudioviz/?.so'
audioviz = require('audioviz')

SIZE = { 1280, 720 }
FFT_SIZE = 3000

fa = audioviz.FrequencyAnalyzer.new(FFT_SIZE)
sa = audioviz.StereoAnalyzer.new()
cs = audioviz.ColorSettings.new()
bss = audioviz.BarStereoSpectrum.new({ {}, SIZE }, cs)
media = audioviz.FfmpegCliBoostMedia.new(arg[1], SIZE)
viz = audioviz.Base.new(SIZE, media)
viz:set_audio_frames_needed(FFT_SIZE)

---@param margin integer
function set_spectrum_margin(margin)
	bss:set_rect({ { margin, margin }, { SIZE[1] - 2 * margin, SIZE[2] - 2 * margin } })
end

set_spectrum_margin(10)
bss:set_left_backwards(true)

if LINUX then
	viz:set_text_font('/usr/share/fonts/TTF/Iosevka-Regular.ttc')
	vcodec = 'h264_vaapi'
elseif WIN32 then
	viz:set_text_font(os.getenv('LocalAppData') .. '\\Microsoft\\Windows\\Fonts\\Iosevka-Regular.ttc')
	vcodec = 'h264_qsv'
end

spectrum_layer = viz:add_layer('spectrum', 0)
spectrum_layer:add_drawable(bss)
spectrum_layer:set_orig_cb(function(_)
	bss:configure_analyzer(sa)
	viz:perform_fft(fa, sa)
	bss:update(sa)
end)

audioviz.start_in_window(viz, 'NEW-test2.lua')
