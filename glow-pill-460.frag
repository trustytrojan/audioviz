#version 460 core

struct Pill
{
	vec2 position, size;
	vec3 color;
};

const Pill pills[] =
{
    {vec2(0.1, 0), vec2(0.01, 0.2), vec3(1, 0.9, 0.9)},
    {vec2(0.13, 0), vec2(0.01, 0.2), vec3(0.5, 0.9, 0.9)}
};

uniform vec2 resolution;

float pill_sdf(const vec2 size, const vec2 position)
{
	vec2 uv = (gl_FragCoord.xy / resolution.xy - 0.5 + position) * vec2(resolution.x / resolution.y, 1);
	vec2 p = abs(uv) - size;
	float d = length(max(p, 0)) + min(max(p.x, p.y), 0);
	float radius = size.x;
	return min(d, min(length(uv - vec2(0, size.y)) - radius, length(uv - vec2(0, -size.y)) - radius));
}

const struct
{
	float strength;
	vec3 color;
} glow = {0.003, vec3(1)};
out vec4 FragColor;

void main()
{
    float minDist = 1e9;
    vec3 color;
	for (int i = 0; i < pills.length(); i++)
	{
        float d = pill_sdf(pills[i].size, pills[i].position);
        if (d < minDist)
		{
            minDist = d;
            color = pills[i].color;
        }
    }
    color *= step(0, -minDist);
	if (color == vec3(0))
		color += clamp((glow.strength / minDist) * glow.color, vec3(0), vec3(glow.color));
    FragColor = vec4(color, 1);
}