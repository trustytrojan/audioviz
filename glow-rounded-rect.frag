#version 330 core

uniform vec2 resolution;

void main()
{
	vec2 uv = gl_FragCoord.xy / resolution.xy; // x: <0, 1>, y: <0, 1>
	uv -= 0.5; // x: <-0.5, 0.5>, y: <-0.5, 0.5>
	uv.x *= resolution.x / resolution.y; // x: <-0.5, 0.5> * aspect ratio, y: <-0.5, 0.5>

	vec2 boxSize = vec2(0.1, 0.2); // Define the size of the rectangle
	vec2 p = abs(uv) - boxSize; // Get the absolute distance of the point from the center of the rectangle
	float d = length(max(p, 0.0)) + min(max(p.x, p.y), 0.0); // Calculate the signed distance

	vec3 color = vec3(step(0., -d)); // create white rectangle with black background

	float glow = 0.01 / d;		// create glow and diminish it with distance
	glow = clamp(glow, 0., 1.); // remove artifacts
	color += glow;				// add glow

	gl_FragColor = vec4(color, 1.0); // output color
}