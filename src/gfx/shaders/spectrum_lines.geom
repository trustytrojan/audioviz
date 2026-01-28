#version 330 compatibility

layout(points) in;
layout(line_strip, max_vertices = 2) out;

uniform float bottom_y;

void main()
{
	vec4 origin = gl_in[0].gl_Position;
	float x = origin.x;
	float height = origin.y;
	float top_y = bottom_y - height;

	vec4 color = gl_in[0].gl_FrontColor;

	// Bottom
	gl_Position = gl_ModelViewProjectionMatrix * vec4(x, bottom_y, 0.0, 1.0);
	gl_FrontColor = color;
	EmitVertex();

	// Top
	gl_Position = gl_ModelViewProjectionMatrix * vec4(x, top_y, 0.0, 1.0);
	gl_FrontColor = color;
	EmitVertex();

	EndPrimitive();
}
