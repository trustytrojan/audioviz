#version 460 core

vec3 hsv2rgb(float h, const float s, const float v)
{
	// Normalize h to the range [0, 1]
	h = mod(h, 1);

	// Ensure h is not negative
	if (h < 0)
		h += 1;

	float r, g, b;

	if (s == 0)
		r = g = b = v;
	else
	{
		float var_h = h * 6;
		if (var_h == 6)
			var_h = 0;

		int var_i = int(var_h);
		float var_1 = v * (1 - s);
		float var_2 = v * (1 - s * (var_h - var_i));
		float var_3 = v * (1 - s * (1 - (var_h - var_i)));

		switch (var_i)
		{
		case 0:
			r = v;
			g = var_3;
			b = var_1;
			break;
		case 1:
			r = var_2;
			g = v;
			b = var_1;
			break;
		case 2:
			r = var_1;
			g = v;
			b = var_3;
			break;
		case 3:
			r = var_1;
			g = var_2;
			b = v;
			break;
		case 4:
			r = var_3;
			g = var_1;
			b = v;
			break;
		case 5:
			r = v;
			g = var_1;
			b = var_2;
			break;
		}
	}

	return vec3(r, g, b);
}

uniform vec2 resolution;

float pill_sdf(const vec2 this_uv, vec2 position, const vec2 size)
{
	position.x *= -1;
	const vec2 uv = this_uv + position;
	const vec2 p = abs(uv) - size;
	const float d = length(max(p, 0)) + min(max(p.x, p.y), 0);
	const float radius = size.x;
	return min(d, min(length(uv - vec2(0, size.y)) - radius, length(uv - vec2(0, -size.y)) - radius));
}

out vec4 FragColor;

void main()
{
	const float aspectRatio = resolution.x / resolution.y;
	const vec2 this_uv = (gl_FragCoord.xy / resolution - 0.5) * vec2(aspectRatio, 1);

	const float glowStrength = 0.003;
	const vec3 glowColor = vec3(1);

	const float startX = -0.5;
	const vec2 pillSize = vec2(0.01, 0.1);
	const float pillSpacing = 0.025;
	// const int numPills = int(1 / (pillSize.x + pillSpacing));
	const int numPills = 5;

	float minDist = 1e9;
	int minIdx = 0;
	vec3 color;

	for (int i = 0; i < numPills; ++i)
	{
		const float d = pill_sdf(this_uv, vec2(startX + i * (pillSize.x + pillSpacing), 0), pillSize);
		if (d < minDist)
		{
			minDist = d;
			minIdx = i;
		}
	}

	// color = hsv2rgb(1 - (float(minIdx) / numPills), 0.9, 1);
	color = vec3(1);

	// decide whether we are inside/outside the pill
	color *= step(0, -minDist);
	
	// only add glow if we are outside the pill
	float alpha;
	if (color == vec3(0))
	{
		alpha = glowStrength / minDist;
		color += clamp(alpha * glowColor, vec3(0), vec3(glowColor));
	}
	{
		alpha = 1;
	}

	FragColor = vec4(color, alpha);
}