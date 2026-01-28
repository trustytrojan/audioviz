#version 330 compatibility

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform float bar_width;
uniform float bottom_y;

void main()
{
	vec4 origin = gl_in[0].gl_Position;
	float x = origin.x;
	float height = origin.y;
	float hw = bar_width * 0.5;
	float top_y = bottom_y - height;

	vec4 color = gl_in[0].gl_FrontColor;

	// Bottom Left
	gl_Position = gl_ModelViewProjectionMatrix * vec4(x - hw, bottom_y, 0.0, 1.0);
	gl_FrontColor = color;
	EmitVertex();

	// Bottom Right
	gl_Position = gl_ModelViewProjectionMatrix * vec4(x + hw, bottom_y, 0.0, 1.0);
	gl_FrontColor = color;
	EmitVertex();

	// Top Left
	gl_Position = gl_ModelViewProjectionMatrix * vec4(x - hw, top_y, 0.0, 1.0);
	gl_FrontColor = color;
	EmitVertex();

	// Top Right
	gl_Position = gl_ModelViewProjectionMatrix * vec4(x + hw, top_y, 0.0, 1.0);
	gl_FrontColor = color;
	EmitVertex();

	EndPrimitive();
}
