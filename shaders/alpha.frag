#version 110

uniform sampler2D image;
uniform vec2 size;
uniform float alpha;

void main()
{
	gl_FragColor = texture2D(image, gl_FragCoord.xy / size);
	gl_FragColor.a = alpha;
}