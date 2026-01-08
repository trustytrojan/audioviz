#version 120

uniform float time;
uniform float frequency;
uniform vec2 amplitude;

void main()
{
	vec4 pos = gl_Vertex;

	// Deterministic-ish pseudo shake based on position + time.
	float t = time * frequency;
	float ox = sin(t + pos.y * 0.05) + sin(t * 1.7 + pos.x * 0.03);
	float oy = cos(t * 1.3 + pos.x * 0.05) + cos(t + pos.y * 0.03);
	vec2 offset = vec2(ox, oy) * (amplitude * 0.5);

	pos.xy += offset;

	gl_Position = gl_ModelViewProjectionMatrix * pos;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_FrontColor = gl_Color;
}
