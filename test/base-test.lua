---@diagnostic disable: lowercase-global, undefined-global
package.cpath = package.cpath .. ';build/luaviz/?.so'
luaviz = require('luaviz')

if not arg[1] then
	print('media file required')
	os.exit(1)
end

-- video window size
SIZE = { 1280, 720 }

-- fft window size
FFT_SIZE = 3000

-- frequency analyzer (fft) logic
fa = luaviz.FrequencyAnalyzer.new(FFT_SIZE)

-- handles fft of stereo audio for you
sa = luaviz.StereoAnalyzer.new()

-- self-explanatory
cs = luaviz.ColorSettings.new()

-- the star of the show
bss = luaviz.BarStereoSpectrum.new({ {}, SIZE }, cs)

-- where the audio is coming from
media = luaviz.FfmpegCliBoostMedia.new(arg[1], SIZE)

-- layering & timing logic
viz = luaviz.Base.new(SIZE, media)

-- tells `viz` to only read FFT_SIZE audio samples from `media` per video frame (default 60fps)
viz:set_audio_frames_needed(FFT_SIZE)

-- iosevka is a great font, use hardware h264 encoder (available on my laptop)
if LINUX then
	font_path = '/usr/share/fonts/TTF/Iosevka-Regular.ttc'
	vcodec = 'h264_vaapi'
elseif WIN32 then
	font_path = os.getenv('LocalAppData') .. '\\Microsoft\\Windows\\Fonts\\Iosevka-Regular.ttc'
	vcodec = 'h264_qsv'
end
viz:set_text_font(font_path)

-- set up song metadata object
font = luaviz.sfFont.new(font_path)
title_text = luaviz.sfText.new(font)
artist_text = luaviz.sfText.new(font)
smd = luaviz.SongMetadataDrawable.new(title_text, artist_text)

-- grab the album cover/art from the audio file
attached_pic = media:attached_pic()

if attached_pic then
	smd:set_album_cover(attached_pic, { 150, 150 })
end

title_text:setStyle(luaviz.sfTextStyle.Bold | luaviz.sfTextStyle.Italic)
title_text:setCharacterSize(24)
title_text:setFillColor({ 255, 255, 255, 150 })
artist_text:setStyle(luaviz.sfTextStyle.Italic)
artist_text:setCharacterSize(24)
artist_text:setFillColor({ 255, 255, 255, 150 })

smd:set_position({ 30, 30 })
smd:use_metadata(media)

---utility function from the c++ code
---@param margin integer
function set_spectrum_margin(margin)
	bss:set_rect({ { margin, margin }, { SIZE[1] - 2 * margin, SIZE[2] - 2 * margin } })
end

set_spectrum_margin(10)
-- make a "mirror" effect
bss:set_left_backwards(true)

if attached_pic then
	bg_spr = luaviz.Sprite.new(attached_pic)
	bg_spr:capture_centered_square_view()
	bg_spr:fill_screen(SIZE)

	bg_layer = viz:add_layer('bg', 0)
	bg_layer:add_drawable(bg_spr)
end

spectrum_layer = viz:add_layer('spectrum', 0)
-- register the object to be drawn by the layer
spectrum_layer:add_drawable(bss)
-- perform IMPORTANT logic before any drawables are drawn
-- (we don't need to draw onto the layer ourselves, hence the unused `_` parameter)
spectrum_layer:set_orig_cb(function(_)
	-- make sure the StereoAnalyzer allocates enough space for the BarStereoSpectrum's bars
	bss:configure_analyzer(sa)
	-- using the configuration in FrequencyAnalyzer, perform fft for both channels and store the result in the StereoAnalyzer
	viz:perform_fft(fa, sa)
	-- update the spectrum bar heights using the fft results
	bss:update(sa)
end)

-- a "final drawable" is something that depends on everything else being drawn first
viz:add_final_drawable(smd)

-- utility function to open a window and start processing `viz`
luaviz.start_in_window(viz, arg[0])
