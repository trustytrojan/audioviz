---@diagnostic disable: lowercase-global, undefined-global
package.cpath = package.cpath .. ';build/luaviz/?.so'
luaviz = require('luaviz')

if not arg[1] then
	print('media file required')
	os.exit(1)
end

SIZE = { 1280, 720 }
SPECTRUM_SIZE = { 720, 1280 // 2 }
FFT_SIZE = 3000
fa = luaviz.FrequencyAnalyzer.new(FFT_SIZE)
sa = luaviz.StereoAnalyzer.new()
cs = luaviz.ColorSettings.new()
LBSD_POS = { 1280 // 2, 0 }
RBSD_POS = { 1280 // 2, 720 }
lbsd = luaviz.BarSpectrumDrawable.new({ LBSD_POS, SPECTRUM_SIZE }, cs)
rbsd = luaviz.BarSpectrumDrawable.new({ RBSD_POS, SPECTRUM_SIZE }, cs)
lcps = luaviz.CircleParticleSystem.new({ { 0, 0 }, { 1280, 720 } }, 45)
rcps = luaviz.CircleParticleSystem.new({ { 0, 0 }, { 1280, 720 } }, 45)
media = luaviz.FfmpegCliBoostMedia.new(arg[1], {})
viz = luaviz.Base.new(SIZE, media)
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

-- assert that either lbsd or rbsd can be used to configure the analyzer
-- aka set the analyzer's buffer size to have enough data for all the spectrum bars
assert(lbsd:bar_count() == rbsd:bar_count(), 'bar counts are not the same!')

-- lbsd:set_debug_rect(true)
-- rbsd:set_debug_rect(true)
-- lcps:set_debug_rect(true)
-- rcps:set_debug_rect(true)

lcps:set_start_side(luaviz.ParticleSystemStartSide.LEFT)
rcps:set_start_side(luaviz.ParticleSystemStartSide.RIGHT)
lbsd:set_backwards(true)
rbsd:set_backwards(true)

lbsd:set_multiplier(6)
rbsd:set_multiplier(6)

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
	bg_layer:add_drawable(bg_spr)
	-- we need to store the effects in variables for now, otherwise they get garbage collected...
	bg_blur = luaviz.Blur.new(7.5, 7.5, 15)
	bg_darken = luaviz.Mult.new(.75)
	bg_layer:add_effect(bg_blur)
	bg_layer:add_effect(bg_darken)
end

particles_layer = viz:add_layer('particles', 0)
particles_layer:set_orig_cb(function(orig_rt)
	orig_rt:clear({ 0, 0, 0, 0 })
	lbsd:configure_analyzer(sa)
	viz:perform_fft(fa, sa)
	lcps:update(sa:left_data())
	rcps:update(sa:right_data())
	orig_rt:draw(lcps)
	orig_rt:draw(rcps)
	orig_rt:display()
end)
particles_layer:set_fx_cb(function(orig_rt, fx_rt, target)
	target:draw(fx_rt:sprite(), luaviz.sfBlendMode.Add)
	target:draw(orig_rt:sprite(), luaviz.sfBlendMode.Add)
end)

particles_blur = luaviz.Blur.new(1, 1, 10)
particles_layer:add_effect(particles_blur)

spectrum_layer = viz:add_layer('spectrum', 0)
spectrum_layer:set_orig_cb(function(orig_rt)
	orig_rt:clear({ 0, 0, 0, 0 })
	lbsd:update(sa:left_data())
	rbsd:update(sa:right_data())
	orig_rt:draw(lbsd, lstates)
	orig_rt:draw(rbsd, rstates)
	orig_rt:display()
end)
spectrum_layer:set_fx_cb(function(orig_rt, fx_rt, target)
	target:draw(fx_rt:sprite(), luaviz.sfBlendMode.Add)
	target:draw(orig_rt:sprite())
end)

-- add subtle glow effect on the spectrum
spectrum_blur = luaviz.Blur.new(3, 3, 10)
spectrum_layer:add_effect(spectrum_blur)

-- metadata
font = luaviz.sfFont.new(font_path)
title_text = luaviz.sfText.new(font)
artist_text = luaviz.sfText.new(font)
smd = luaviz.SongMetadataDrawable.new(title_text, artist_text)

if attached_pic then
	smd:set_album_cover(attached_pic, { 150, 150 })
end

title_text:setStyle(luaviz.sfTextStyle.Bold | luaviz.sfTextStyle.Italic)
title_text:setCharacterSize(24)
title_text:setFillColor({ 255, 255, 255, 150 })
artist_text:setStyle(luaviz.sfTextStyle.Italic)
artist_text:setCharacterSize(24)
artist_text:setFillColor({ 255, 255, 255, 150 })

-- centered in the window... doesn't really look good without some kind of contrasting effect
smd:set_position({ 640 - 200, 360 - 75 })

-- smd:set_position({ 30, 30 })
smd:use_metadata(media)

viz:add_final_drawable(smd)

viz:start_in_window(arg[0])
