#version 330 compatibility

layout(points) in;
layout(line_strip, max_vertices = 2) out;

uniform vec2 size;
uniform float base_radius;
uniform float max_radius;
uniform float angle_start;
uniform float angle_span;
uniform float warping_factor;

uniform float bottom_y;

void emit_polar(vec2 world_pos)
{
	// Treat the drawable's origin as its "center" to be moved onto the polar arc.
	// We use the bottom of the bar as the reference origin for warping logic consistency.
	vec4 world_origin = vec4(world_pos.x, bottom_y, 0.0, 1.0);

	// Preserve the drawable's local shape/orientation as an offset from its origin.
	vec2 target_ref = mix(world_origin.xy, world_pos.xy, warping_factor);

	// Map the reference point onto the polar coordinates
	float angle = angle_start + (target_ref.x / size.x) * angle_span;

	// Normalize height factor (scene coordinate Y -> [0, 1])
	float height_factor = (size.y - target_ref.y) / size.y;

	// Linearly interpolate radius
	float radius = base_radius + (height_factor * (max_radius - base_radius));

	vec2 polar_pos;
	polar_pos.x = radius * cos(angle);
	polar_pos.y = radius * sin(angle);

	// Preserve local offset relative to reference point
	vec2 offset = world_pos.xy - target_ref;

	// Convert to NDC (-1 to 1)
	float aspect = size.x / size.y;
	vec2 center_ndc = polar_pos / (size.y * 0.5);
	vec2 offset_ndc = offset / (size.y * 0.5);

	// Apply aspect ratio correction
	center_ndc.x /= aspect;
	offset_ndc.x /= aspect;

	gl_Position = vec4(center_ndc + offset_ndc, 0.0, 1.0);
	gl_FrontColor = gl_in[0].gl_FrontColor;
	EmitVertex();
}

void main()
{
	float x = gl_in[0].gl_Position.x;
	float height = gl_in[0].gl_Position.y;
	float top_y = bottom_y - height;

	emit_polar(vec2(x, bottom_y)); // Bottom
	emit_polar(vec2(x, top_y));    // Top

	EndPrimitive();
}
