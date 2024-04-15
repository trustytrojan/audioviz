#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D textureSampler;
uniform vec2 resolution; // pass the resolution of your texture from your application

// A simple hash function
vec2 hash(vec2 p)
{
	p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
	return fract(sin(p) * 43758.5453);
}

void main()
{
	vec4 texColor = texture(textureSampler, TexCoords);

	// Generate three fixed positions
	vec2 pos1 = hash(vec2(1.0, 1.0));
	vec2 pos2 = hash(vec2(2.0, 2.0));
	vec2 pos3 = hash(vec2(3.0, 3.0));

	// If the current pixel is at one of these positions, color it green
	if (TexCoords == pos1 / resolution || TexCoords == pos2 / resolution || TexCoords == pos3 / resolution)
	{
		FragColor = vec4(0.0, 1.0, 0.0, 1.0); // green color
	}
	else
	{
		FragColor = texColor;
	}
}