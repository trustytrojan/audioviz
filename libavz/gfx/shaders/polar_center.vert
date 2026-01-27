#version 120

uniform vec2 size;          // 1280, 720
uniform float base_radius;  // pixels
uniform float max_radius;   // pixels - half of size.y
uniform float angle_start;  // radians
uniform float angle_span;   // radians

void main()
{
    // Model-space -> world-space
    vec4 world_pos = gl_ModelViewMatrix * gl_Vertex;

    // Treat the drawable's origin as its "center" to be moved onto the polar arc.
    vec4 world_origin = gl_ModelViewMatrix * vec4(0.0, 0.0, 0.0, 1.0);

    // Preserve the drawable's local shape/orientation as an offset from its origin.
    vec2 offset = world_pos.xy - world_origin.xy;

    // Map the origin onto the polar coordinates (angle based on origin.x, radius from origin.y).
    float angle = angle_start + (world_origin.x / size.x) * angle_span;
    float height_factor = (size.y - world_origin.y) / size.y;
    height_factor = clamp(height_factor, 0.0, 1.0);
    float radius = base_radius + (height_factor * (max_radius - base_radius));

    vec2 polar_center;
    polar_center.x = radius * cos(angle);
    polar_center.y = radius * sin(angle);

    // Convert both the center and the preserved offset into NDC.
    float aspect = size.x / size.y;

    vec2 center_ndc = polar_center / (size.y * 0.5);
    center_ndc.x /= aspect;

    vec2 offset_ndc = offset / (size.y * 0.5);
    offset_ndc.x /= aspect;

    vec2 ndc_pos = center_ndc + offset_ndc;

    gl_Position = vec4(ndc_pos, 0.0, 1.0);

    gl_FrontColor = gl_Color;
    gl_TexCoord[0] = gl_MultiTexCoord0;
}
