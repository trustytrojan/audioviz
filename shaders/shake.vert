#version 120

uniform float time;

// x=Low, y=Mid, z=High
uniform vec3 frequencies;
uniform vec3 amplitudes;

// Helper: Generates a chaotic, non-circular shake for a specific band
vec2 get_band_shake(float t, float freq, float amp)
{
    t *= 5. * freq;
    float x = sin(t) + cos(t * 1.73);
    float y = sin(t * 1.41) + cos(t * 2.39);
    return vec2(x, y) * amp;
}

void main()
{
    vec4 pos = gl_Vertex;

    // Accumulate shake from all 3 bands
    vec2 total_offset = vec2(0.0);

    // Band 1: Low Bass (Heavy, slower offset)
    total_offset += get_band_shake(time, frequencies.x, amplitudes.x);

    // Band 2: Mid Bass (Tight, fast offset)
    // We offset 'time' by 100.0 to make sure Mid and Low don't sync up
    total_offset += get_band_shake(time + 100.0, frequencies.y, amplitudes.y * 0.66);

    // Band 3: High Bass (Vibration)
    // Offset time again.
    total_offset += get_band_shake(time + 200.0, frequencies.z, amplitudes.z * 0.33);

    pos.xy += total_offset;

    gl_Position = gl_ModelViewProjectionMatrix * pos;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_FrontColor = gl_Color;
}