---@diagnostic disable: lowercase-global, undefined-global

size = { 1280, 720 }
fft_size = 3000
fa = tt.FrequencyAnalyzer.new(fft_size)
sa = tt.StereoAnalyzer.new()
cs = viz.ColorSettings.new()
ss = viz.StereoSpectrum.new({ {}, size }, cs)
media = FfmpegCliBoostMedia.new(args.media_url, size)
_viz = base_audioviz.new(size, media)
_viz:set_audio_frames_needed(fft_size)

---@param margin integer
function set_spectrum_margin(margin)
	ss:set_rect({ { margin, margin }, { size[1] - 2 * margin, size[2] - 2 * margin } })
end

ss:set_left_backwards(true)
sa:resize(ss:get_bar_count())

if LINUX then
	_viz:set_text_font('/usr/share/fonts/TTF/Iosevka-Regular.ttc')
	vcodec = 'h264_vaapi'
elseif WIN32 then
	_viz:set_text_font(os.getenv('LocalAppData') .. '\\Microsoft\\Windows\\Fonts\\Iosevka-Regular.ttc')
	vcodec = 'h264_qsv'
end

spectrum_layer = _viz:add_layer('spectrum', 0)
spectrum_layer:set_orig_cb(function(orig_rt)
	_viz:perform_fft(fa, sa)
	ss:update(sa)
	orig_rt:clear({ 0, 0, 0, 0 })
	orig_rt:draw(ss)
	orig_rt:display()
end)
spectrum_layer:set_fx_cb(viz.Layer.DRAW_FX_RT)

start_in_window(_viz)
