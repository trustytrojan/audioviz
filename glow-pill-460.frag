#version 460 core

vec3 hsv2rgb(float h, const float s, const float v)
{
	// Normalize h to the range [0, 1]
	h = mod(h, 1);

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

vec2 this_uv;

// draws a line segment with a thickness/radius
// a: first point
// b: second point
// r: radius/thickness of time
float ligne_sdf(vec2 a, vec2 b, float r)
{
	const vec2 pa = this_uv - a, ba = b - a;
	const float h = clamp(dot(pa, ba) / dot(ba, ba), 0, 1);
	return length(pa - ba * h) - r;
}

uniform vec2 resolution;
out vec4 FragColor;

void main()
{
	// step 1: calculate and initialize this_uv
	{
		const float aspect_ratio = resolution.x / resolution.y;

		// normalize this fragment's coordinates to [0, 1] along both axes,
		// 0 being the (left|top)most pixel per axis
		this_uv = gl_FragCoord.xy / resolution;

		// move origin to center of the screen such that
		// coordinates are in the range [-0.5, 0.5]
		this_uv -= 0.5;

		// to make life easier, double the range to [-1, 1]
		this_uv *= 2;

		// lastly, don't forget to scale x-axis by aspect_ratio.
		// otherwise, on a non-sqaure aspect ratio, everything will be stretched.
		this_uv.x *= aspect_ratio;
	}

	// step 2: find distance to closest pill, and get its color
	// note: do not put this code in a function; it does not get inlined (~2000 fps loss)
	vec3 color;
	float minDist;
	{
		const float startX = -0.95;
		const vec2 pillSize = vec2(0.02, 0.5);
		const float pillSpacing = 0.05;

		// 2 since the coordinate range is [-1, 1] per axis
		// might need to revise the denominator though
		const int numPills = int(2 / (pillSize.x + pillSpacing));

		int i = 0;
		for (float prevDist = 1e9, d; i < numPills; ++i)
		{
			const float x = startX + i * (pillSize.x + pillSpacing);

			// ligne_sdf is getting inlined by the compiler!
			// tested this by unrolling the function right here,
			// and saw no noticeable performance improvement.
			d = ligne_sdf(vec2(x, -0.95), vec2(x, 0), pillSize.x);

			if (d > prevDist)
			{
				minDist = prevDist;
				break;
			}

			prevDist = d;
		}

		color = hsv2rgb(float(i) / numPills, 0.9, 1);
	}

	// step 3: coloring
	{
		// decide whether we are inside/outside the pill
		color *= step(0, -minDist);

		// subtly blend glow with background if we are outside the pill
		float alpha = 1;
		if (color == vec3(0))
		{
			const float glowStrength = 0.003;
			const vec3 glowColor = vec3(1);

			// decrease glow as we go away from the pill
			alpha = glowStrength / minDist;

			// glow amount calculation
			color += clamp(alpha * glowColor, vec3(0), vec3(glowColor));
		}

		FragColor = vec4(color, alpha);
	}
}