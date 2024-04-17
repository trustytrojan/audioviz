#version 460 core

uniform vec2 resolution;

struct Pill
{
	vec2 position, size;
	vec3 color;
};

const Pill pills[] =
{
    {vec2(0.1, 0), vec2(0.01, 0.2), vec3(1, 0.9, 0.9)},
    {vec2(0.2, 0), vec2(0.01, 0.2), vec3(0.5, 0.9, 0.9)}
};

float pill_sdf(const Pill pill)
{
	vec2 uv = (gl_FragCoord.xy / resolution.xy - 0.5 + pill.position) * vec2(resolution.x / resolution.y, 1);
	vec2 p = abs(uv) - pill.size;
	float d = length(max(p, 0)) + min(max(p.x, p.y), 0);
	float radius = pill.size.x;
	d = min(d, min(length(uv - vec2(0, pill.size.y)) - radius, length(uv - vec2(0, -pill.size.y)) - radius));
	return d;
}

const float glowStrength = 0.015;

out vec4 FragColor;

void main()
{
    float minDist = 1e9;
    vec3 pillColor, glowColor = vec3(0);
	for (int i = 0; i < pills.length(); i++)
	{
        float d = pill_sdf(pills[i]);
        if (d < minDist)
		{
            minDist = d;
            pillColor = pills[i].color;
        }
		glowColor += cla1mp((glowStrength / d) * pills[i].color, vec3(0), vec3(pills[i].color));
    }

    // vec3 glowColor = vec3(1);
    vec3 color = pillColor * step(0, -minDist);
	if (color == vec3(0))
		// color += clamp((glowStrength / minDist) * glowColor, vec3(0), vec3(glowColor));
		color += glowColor;
    FragColor = vec4(color, 1);
}