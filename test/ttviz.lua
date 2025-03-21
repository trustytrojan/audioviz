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
-- fa:set_accum_method('sum')

-- handles fft of stereo audio for you
sa = luaviz.StereoAnalyzer.new()

-- self-explanatory
cs = luaviz.ColorSettings.new()

-- the star of the show
bss = luaviz.BarStereoSpectrum.new({ {}, SIZE }, cs)
-- bss:set_bar_width(1)
-- bss:set_bar_spacing(0)

-- particles!!!!!!!!!!!
cps = luaviz.CircleParticleSystem.new({ {}, SIZE }, 50)

-- where the audio is coming from
media = luaviz.FfmpegCliBoostMedia.new(arg[1], SIZE)

-- layering & timing logic
viz = luaviz.Base.new(SIZE, media)
framerate = viz:get_framerate()

-- tells `viz` to only read FFT_SIZE audio samples from `media` per video frame (default 60fps)
viz:set_audio_frames_needed(FFT_SIZE)

-- iosevka is a great font, use hardware h264 encoder (available on my laptop)
if luaviz.os == 'linux' then
	font_path = '/usr/share/fonts/TTF/Iosevka-Regular.ttc'
	vcodec = 'h264_vaapi'
elseif luaviz.os == 'windows' then
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
	-- we need to store the effects in variables for now, otherwise they get garbage collected...
	bg_blur = luaviz.Blur.new(7.5, 7.5, 15)
	bg_darken = luaviz.Mult.new(.75)
	bg_layer:add_effect(bg_blur)
	bg_layer:add_effect(bg_darken)
end

function do_fft_safely()
	-- make sure the StereoAnalyzer allocates enough space for the BarStereoSpectrum's bars
	bss:configure_analyzer(sa)
	-- using the configuration in FrequencyAnalyzer, perform fft for both channels and store the result in the StereoAnalyzer
	viz:perform_fft(fa, sa)
end

particles_layer = viz:add_layer('particles', 0)
particles_layer:add_drawable(cps)
particles_blur = luaviz.Blur.new(1, 1, 10)
particles_layer:add_effect(particles_blur)

particles_layer:set_orig_cb(function(_)
	do_fft_safely()
	if framerate == 60 then
		cps:update(sa)
	end
	-- handle other framerates, check ttviz::layers_init() for reference
end)

particles_layer:set_fx_cb(function(orig_rt, fx_rt, target)
	target:draw(fx_rt:sprite(), luaviz.sfBlendMode.Add)
	target:draw(orig_rt:sprite(), luaviz.sfBlendMode.Add)
end)

spectrum_layer = viz:add_layer('spectrum', 0)

-- register the object to be drawn by the layer
spectrum_layer:add_drawable(bss)

-- perform IMPORTANT logic before any drawables are drawn
-- (we don't need to draw onto the layer ourselves, hence the unused `_` parameter)
spectrum_layer:set_orig_cb(function(_)
	-- update the spectrum bar heights using the fft results
	bss:update(sa)
end)

-- blend the layer onto the final picture by adding colors (involves transparency)
spectrum_layer:set_fx_cb(function(orig_rt, fx_rt, target)
	target:draw(fx_rt:sprite(), luaviz.sfBlendMode.Add)
	target:draw(orig_rt:sprite())
end)

-- add subtle glow effect on the spectrum
spectrum_blur = luaviz.Blur.new(3, 3, 10)
spectrum_layer:add_effect(spectrum_blur)

-- a "final drawable" is something that depends on everything else being drawn first
viz:add_final_drawable(smd)

-- watch the viz!!!!!!!!!!!!!!!
viz:start_in_window(arg[0])
