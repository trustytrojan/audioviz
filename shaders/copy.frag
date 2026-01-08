#version 110

uniform sampler2D image;
uniform vec2 size; // in <1.30 we have to pass the size manually

void main()
{
	gl_FragColor = texture2D(image, gl_FragCoord.xy / size);
}
