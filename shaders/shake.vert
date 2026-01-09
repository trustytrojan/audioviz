#version 120

uniform float time;
uniform float frequency;
uniform vec2 amplitude;

void main()
{
	vec4 pos = gl_Vertex;

	// Deterministic-ish pseudo shake based on time only (not position).
	// This way all vertices move together, preserving the object's shape.
	float t = time * frequency;
	float ox = sin(t) + sin(t * 1.7);
	float oy = cos(t * 1.3) + cos(t);
	vec2 offset = vec2(ox, oy) * amplitude;

	pos.xy += offset;

	gl_Position = gl_ModelViewProjectionMatrix * pos;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_FrontColor = gl_Color;
}
