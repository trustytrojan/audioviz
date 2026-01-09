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

SIZE = { 1280, 720 }
SPECTRUM_SIZE = { 720, 1280 }
FFT_SIZE = 3000
fa = luaviz.FrequencyAnalyzer.new(FFT_SIZE)
sa = luaviz.StereoAnalyzer.new()
cs = luaviz.ColorSettings.new()
LBSD_POS = { 1280, 0 }
RBSD_POS = { 0, 720 }
lbsd = luaviz.BarSpectrumDrawable.new({ LBSD_POS, SPECTRUM_SIZE }, cs)
rbsd = luaviz.BarSpectrumDrawable.new({ RBSD_POS, SPECTRUM_SIZE }, cs)
lcps = luaviz.CircleParticleSystem.new({ { 0, 0 }, { 1280, 720 } }, 45)
rcps = luaviz.CircleParticleSystem.new({ { 0, 0 }, { 1280, 720 } }, 45)
media = luaviz.FfmpegPopenMedia.new(arg[1], {})
viz = luaviz.Base.new(SIZE, media)
viz:set_audio_frames_needed(FFT_SIZE)
viz:set_timing_text_enabled(true)

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

-- assert that either lbsd or rbsd can be used to configure the analyzer
-- aka set the analyzer's buffer size to have enough data for all the spectrum bars
assert(lbsd:bar_count() == rbsd:bar_count(), 'bar counts are not the same!')

lcps:set_start_side(luaviz.ParticleSystemStartSide.LEFT)
rcps:set_start_side(luaviz.ParticleSystemStartSide.RIGHT)
lbsd:set_backwards(true)
rbsd:set_backwards(true)
lbsd:set_bar_width(5)
rbsd:set_bar_width(5)
lbsd:set_bar_spacing(2)
rbsd:set_bar_spacing(2)

lstates = luaviz.sfRenderStates.new()
lstates.transform:rotateDegrees(90, LBSD_POS)

rstates = luaviz.sfRenderStates.new()
rstates.transform:rotateDegrees(-90, RBSD_POS)

-- grab the album cover/art from the audio file
attached_pic = media:attached_pic()

if attached_pic then
	bg_spr = luaviz.Sprite.new(attached_pic)
	bg_spr:capture_centered_square_view()
	bg_spr:fill_screen(SIZE)

	bg_layer = viz:add_layer('bg', 0)
	bg_layer:add_draw(bg_spr)
	-- we need to store the effects in variables for now, otherwise they get garbage collected...
	bg_blur = luaviz.Blur.new(7.5, 7.5, 15)
	bg_darken = luaviz.Mult.new(.75)
	bg_layer:add_effect(bg_blur)
	bg_layer:add_effect(bg_darken)

	-- make spectrum match bg color
	cs:set_mode(luaviz.ColorMode.SOLID)
	-- cs:set_solid_color({ 199, 220, 241, 255 })
	-- cs:set_solid_color({ 90, 75, 94, 255 })
end

updopts = luaviz.ParticleSystemUpdateOptions.new()
updopts.multiplier = 1.25

-- lcps:set_color({ 80, 191, 122, 255 })
-- rcps:set_color({ 80, 191, 122, 255 })

particles_layer = viz:add_layer('particles', 0)
particles_layer:set_orig_cb(function(orig_rt)
	orig_rt:clear({ 0, 0, 0, 0 })
	lbsd:configure_analyzer(sa)
	viz:perform_fft(fa, sa)

	luaviz.Shake_setParameters(sa, samplerate, FFT_SIZE, 1000)

	lcps:update(sa:left_data(), updopts)
	rcps:update(sa:right_data(), updopts)
	orig_rt:draw(lcps)
	orig_rt:draw(rcps)
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
	orig_rt:clear({ 0, 0, 0, 0 })
	-- cs:set_solid_color({ 80, 191, 122, 255 })
	-- cs:set_solid_color(luaviz.sfColors.Magenta)
	lbsd:update(sa:left_data())
	rbsd:update(sa:right_data())
	orig_rt:draw(lbsd, lstates)
	orig_rt:draw(rbsd, rstates)
	orig_rt:display()
end)
spectrum_layer:set_fx_cb(function(orig_rt, fx_rt, target)
	target:draw(fx_rt:sprite(), luaviz.GreatAmazingBlendMode)
	target:draw(orig_rt:sprite())

	-- glow color different from spectrum color:
	-- cs:set_solid_color({ 90, 75, 94, 255 })
	-- cs:set_solid_color(luaviz.sfColors.White)
	-- lbsd:update_colors()
	-- rbsd:update_colors()
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

-- THE SHAKE SHADER IS GOING TO ONLY PUSH IT DOWN
-- BECAUSE IT'S USING A LESS PRECISE SPECTRUM!!!!!
-- add more control to FrequencyAnalyzer to fix this problem!

final_rs = luaviz.sfRenderStates.new()
final_rs.shader = luaviz.Shake_getShader()
viz:add_final_drawable2(smd, final_rs)

viz:start_in_window(media, arg[0])
-- viz:encode(media, 'out.mp4', vcodec, 'copy')
