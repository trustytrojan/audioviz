#version 120

uniform vec2 size;
uniform float base_radius;
uniform float max_radius;
uniform float angle_start;
uniform float angle_span;

// 0.0 = No warping (translate only), 1.0 = Full polar warp
uniform float warping_factor;

void main()
{
    // 1. Get world positions
    vec4 world_pos = gl_ModelViewMatrix * gl_Vertex;
    vec4 world_origin = gl_ModelViewMatrix * vec4(0.0, 0.0, 0.0, 1.0);

    // 2. Calculate the target reference point
    // We mix between the specific vertex and the object's origin
    vec2 target_ref = mix(world_origin.xy, world_pos.xy, warping_factor);

    // 3. Map the target reference onto polar coordinates
    float angle = angle_start + (target_ref.x / size.x) * angle_span;
    float height_factor = (size.y - target_ref.y) / size.y;

    // Unclamped to allow buffer zones as previously discussed
    float radius = base_radius + (height_factor * (max_radius - base_radius));

    vec2 polar_pos;
    polar_pos.x = radius * cos(angle);
    polar_pos.y = radius * sin(angle);

    // 4. Handle the Offset (only applies when warping_factor < 1.0)
    // If warping_factor is 1, this offset becomes 0.
    vec2 offset = world_pos.xy - target_ref;

    // 5. Conversion to NDC
    float aspect = size.x / size.y;
    vec2 center_ndc = polar_pos / (size.y * 0.5);
    vec2 offset_ndc = offset / (size.y * 0.5);

    center_ndc.x /= aspect;
    offset_ndc.x /= aspect;

    gl_Position = vec4(center_ndc + offset_ndc, 0.0, 1.0);

    gl_FrontColor = gl_Color;
    gl_TexCoord[0] = gl_MultiTexCoord0;
}