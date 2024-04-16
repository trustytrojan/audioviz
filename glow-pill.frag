#version 330 core

uniform vec2 resolution;

// sdf for drawing the pill
float pill_sdf(float x, float y, float width, float height)
{
	vec2 uv = gl_FragCoord.xy / resolution.xy;
	uv -= 0.5; // make (0, 0) the center
	uv += vec2(x, y); // add (x, y) offset
	uv.x *= resolution.x / resolution.y; // account for aspect ratio

	vec2 rectSize = vec2(width, height); // Define the size of the rectangle
	vec2 p = abs(uv) - rectSize; // Get the absolute distance of the point from the center of the rectangle

	// Calculate the signed distance
	float d = length(max(p, 0)) + min(max(p.x, p.y), 0);

	// Add the semicircles on the top and bottom
	float radius = width; // Radius of the semicircles
	float distToTopSemicircle = length(uv - vec2(0, rectSize.y)) - radius;
	float distToBottomSemicircle = length(uv - vec2(0, -rectSize.y)) - radius;
	return min(d, min(distToTopSemicircle, distToBottomSemicircle));
}

const vec3 pillColor = vec3(1, 0.9, 0.9);
const vec3 glowColor = pillColor;
const float glowStrength = 0.0015;

void main()
{
	float p1 = pill_sdf(-0.2, 0, 0.1, 0.2);
	float p2 = pill_sdf(0.2, 0, 0.1, 0.2);
	float d = min(p1, p2);
	vec3 color = pillColor * step(0, -d); // results in either black or pillColor
	color += clamp((glowStrength / d) * glowColor, vec3(0), vec3(1));
	gl_FragColor = vec4(color, 1);
}