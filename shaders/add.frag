#version 120

uniform sampler2D image;
uniform vec2 size; // in 1.20 we have to pass the size manually
uniform float addend;

void main()
{
	gl_FragColor = texture2D(image, gl_FragCoord.xy / size) + addend;
}