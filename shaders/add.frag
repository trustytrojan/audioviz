#version 120

uniform sampler2D image;
uniform vec2 size;
uniform float addend;

void main()
{
	vec2 coord = gl_FragCoord.xy;
	gl_FragColor = texture2D(image, coord / size) + addend;
}