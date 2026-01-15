#version 120

uniform vec2 size;          // 1280, 720
uniform float base_radius;  // e.g., 100.0 (pixels)
uniform float max_radius;   // e.g., 360.0 (pixels) - half of size.y
uniform float angle_start;  // Starting angle in radians (default: PI/2 for top)
uniform float angle_span;   // Angular span in radians (default: 2*PI for full circle)

const float PI = 3.14159265359;

void main()
{
    // 1. Get position from ModelView
    vec4 worldPos = gl_ModelViewMatrix * gl_Vertex;

    // 2. Map the scene X to the angle range
    float angle = angle_start + (worldPos.x / size.x) * angle_span;

    // 3. NORMALIZE THE BAR HEIGHT (0.0 to 1.0)
    // We calculate how far "up" the bar the current vertex is.
    // If your bars are at the bottom of the scene, use (size.y - worldPos.y) / size.y;
    float height_factor = (size.y - worldPos.y) / size.y;
    height_factor = clamp(height_factor, 0.0, 1.0); // Safety first

    // 4. LINEARLY INTERPOLATE RADIUS
    // This "squeezes" the entire scene height into the gap between
    // the inner circle and the screen edge.
    float radius = base_radius + (height_factor * (max_radius - base_radius));

    // 5. Convert to Cartesian
    vec2 polar_pos;
    polar_pos.x = radius * cos(angle);
    polar_pos.y = radius * sin(angle);

    // 6. Aspect Ratio & NDC Conversion
    float aspect = size.x / size.y;
    
    // Convert to NDC (-1 to 1). 
    // We use (size.y * 0.5) as our "1.0" unit.
    vec2 ndc_pos = polar_pos / (size.y * 0.5);

    // 1:1 Aspect Correction
    ndc_pos.x /= aspect;

    gl_Position = vec4(ndc_pos, 0.0, 1.0);
    
    gl_FrontColor = gl_Color;
    gl_TexCoord[0] = gl_MultiTexCoord0;
}