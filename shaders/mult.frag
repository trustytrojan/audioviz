#version 460 core

uniform sampler2D image;
uniform float factor;
out vec4 color;

void main()
{
	const vec2 coord = gl_FragCoord.xy;
	const ivec2 size = textureSize(image, 0);
	color = texture(image, coord / size);
	color.rgb *= factor;
}