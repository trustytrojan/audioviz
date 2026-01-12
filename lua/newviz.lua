---@diagnostic disable: lowercase-global, undefined-global

-- if running off standalone lua executable
if not luaviz and package and package.cpath then
	package.cpath = package.cpath .. ';build/luaviz/?.so;build/luaviz/?.dll'
	luaviz = require('luaviz')
end

if not arg[1] then
	print('media file required')
	os.exit(1)
end

-- constants
FRAMERATE = 240
SIZE = { 1280, 720 }
UNROTATED_SIZE = { 720, 1280 }
FFT_SIZE = 3000
LEFT_POS = { 1280, 0 }
RIGHT_POS = { 0, 720 }
PARTICLE_COUNT = 45
START_TIME_SEC = 20

-- audio analyzer objects
fa = luaviz.FrequencyAnalyzer.new(FFT_SIZE)
sa = luaviz.StereoAnalyzer.new()

-- scene objects
cs = luaviz.ColorSettings.new()
cs:set_mode(luaviz.ColorMode.SOLID)
lbsd = luaviz.BarSpectrumDrawable.new({ LEFT_POS, UNROTATED_SIZE }, cs)
rbsd = luaviz.BarSpectrumDrawable.new({ RIGHT_POS, UNROTATED_SIZE }, cs)
lcps = luaviz.CircleParticleSystem.new({ LEFT_POS, UNROTATED_SIZE }, PARTICLE_COUNT, FRAMERATE)
rcps = luaviz.CircleParticleSystem.new({ RIGHT_POS, UNROTATED_SIZE }, PARTICLE_COUNT, FRAMERATE)

-- media provider
media = luaviz.FfmpegPopenMedia.new(arg[1], START_TIME_SEC)

-- visualizer
viz = luaviz.Base.new(SIZE, media)
viz:set_audio_frames_needed(FFT_SIZE)
-- viz:set_timing_text_enabled(true)
viz:set_framerate(FRAMERATE)

samplerate = media:audio_sample_rate()

-- iosevka is a great font, use hardware h264 encoder (available on my laptop)
if luaviz.os == 'linux' then
	font_path = '/usr/share/fonts/TTF/Iosevka-Regular.ttc'
	vcodec = 'h264_vaapi'
elseif luaviz.os == 'windows' then
	font_path = os.getenv('LocalAppData') .. '\\Microsoft\\Windows\\Fonts\\IosevkaFixed-Regular.ttf'
	vcodec = 'h264_qsv'
end
viz:set_text_font(font_path)

-- sanity check
assert(lbsd:bar_count() == rbsd:bar_count(), 'bar counts are not the same!')

lbsd:set_backwards(true)
rbsd:set_backwards(true)
lbsd:set_bar_width(5)
rbsd:set_bar_width(5)
lbsd:set_bar_spacing(2)
rbsd:set_bar_spacing(2)
lbsd:set_multiplier(3)
rbsd:set_multiplier(3)

-- sfRenderStates containing the rotations for the spectrums/particle systems
lstates = luaviz.sfRenderStates.new()
lstates.transform:rotateDegrees(90, LEFT_POS)
rstates = luaviz.sfRenderStates.new()
rstates.transform:rotateDegrees(-90, RIGHT_POS)

-- use the album cover/art from the audio file, if available
attached_pic = media:attached_pic()
if attached_pic then
	bg_spr = luaviz.Sprite.new(attached_pic)

	-- zooms it in assuming it is a square image within a 16:9 image (youtube music)
	bg_spr:capture_centered_square_view()
	bg_spr:fill_screen(SIZE)

	bg_layer = viz:add_layer('bg', 0)
	bg_layer:add_draw(bg_spr)
	-- effects must be kept in variables, otherwise they get garbage collected...
	bg_blur = luaviz.Blur.new(7.5, 7.5, 15)
	bg_darken = luaviz.Mult.new(.75)
	bg_layer:add_effect(bg_blur)
	bg_layer:add_effect(bg_darken)
end

-- particle_color = { 255, 150, 150, 255 }
-- lcps:set_color(particle_color)
-- rcps:set_color(particle_color)

particles_layer = viz:add_layer('particles', 0)
particles_layer:set_orig_cb(function(orig_rt)
	orig_rt:clear(luaviz.sfColors.Transparent)
	viz:perform_fft(fa, sa)

	luaviz.Shake_setParameters(sa, samplerate, FFT_SIZE, 15, ac_spr_center)

	lcps:update(sa, samplerate, FFT_SIZE, 0)
	rcps:update(sa, samplerate, FFT_SIZE, 1)
	orig_rt:draw(lcps, lstates)
	orig_rt:draw(rcps, rstates)
	orig_rt:display()
end)
particles_layer:set_fx_cb(function(orig_rt, fx_rt, target)
	target:draw(fx_rt:sprite(), luaviz.sfBlendModes.Add)
	target:draw(orig_rt:sprite(), luaviz.sfBlendModes.Add)
end)

particles_blur = luaviz.Blur.new(1, 1, 10)
particles_layer:add_effect(particles_blur)

spectrum_layer = viz:add_layer('spectrum', 0)
spectrum_layer:set_orig_cb(function(orig_rt)
	-- update the bar heights using the audio analyzers, specifying the channel index (0 and 1 for stereo)
	lbsd:update(fa, sa, 0)
	rbsd:update(fa, sa, 1)

	orig_rt:clear(luaviz.sfColors.Transparent)

	-- -- change color here as this will be the glow color
	-- cs:set_solid_color(luaviz.sfColors.Red)
	-- lbsd:update_bar_colors()
	-- rbsd:update_bar_colors()

	orig_rt:draw(lbsd, lstates)
	orig_rt:draw(rbsd, rstates)
	orig_rt:display()
end)
spectrum_layer:set_fx_cb(function(orig_rt, fx_rt, target)
	target:draw(fx_rt:sprite(), luaviz.GreatAmazingBlendMode)
	target:draw(orig_rt:sprite())

	-- from above, here we draw the spectrum again with a different color
	-- this gives the spectrum an underglow!
	-- cs:set_solid_color(luaviz.sfColors.Transparent)
	-- lbsd:update_bar_colors()
	-- rbsd:update_bar_colors()

	-- target:draw(lbsd, lstates)
	-- target:draw(rbsd, rstates)
end)

-- add subtle glow effect on the spectrum
spectrum_blur = luaviz.Blur.new(3, 3, 10)
spectrum_layer:add_effect(spectrum_blur)

-- metadata
font = luaviz.sfFont.new(font_path)
title_text = luaviz.sfText.new(font)
artist_text = luaviz.sfText.new(font)
smd = luaviz.SongMetadataDrawable.new(title_text, artist_text)

AC_SIZE = { 250, 250 }
if attached_pic then
	smd:set_album_cover(attached_pic, AC_SIZE)
end

title_text:setStyle(luaviz.sfTextStyle.Bold | luaviz.sfTextStyle.Italic)
title_text:setCharacterSize(24)
title_text:setFillColor({ 255, 255, 255, 150 })
artist_text:setStyle(luaviz.sfTextStyle.Italic)
artist_text:setCharacterSize(24)
artist_text:setFillColor({ 255, 255, 255, 150 })

-- centered in the window... doesn't really look good without some kind of contrasting effect
smd:set_position({ SIZE[1] // 2 - AC_SIZE[1] // 2, SIZE[2] // 2 - AC_SIZE[2] // 2 })
-- smd:set_position({ 30, 30 })
smd:use_metadata(media)
smd:set_text_pos(luaviz.SMDTextPosition.BOTTOM)
ac_spr_center = smd:get_ac_spr_center()

shake_rs = luaviz.sfRenderStates.new()
shake_rs.shader = luaviz.Shake_getShader()
viz:add_final_drawable2(smd, shake_rs)

viz:start_in_window(media, arg[0])
-- viz:encode(media, 'luaviz-out.mp4', vcodec, 'copy')
