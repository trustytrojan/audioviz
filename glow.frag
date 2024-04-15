// adapted from blur.frag to glow using a user-defined color instead of a nearby color

#version 330 core

uniform sampler2D image;
uniform vec2 resolution;
uniform vec2 direction;
uniform bool flip;
uniform vec3 glowColor;

void main() {
	vec2 uv = vec2(gl_FragCoord.xy / resolution);
	if (flip) uv.y = 1.0 - uv.y;
	vec4 color = vec4(0.0);
    vec2 off1 = vec2(1.411764705882353) * direction;
    vec2 off2 = vec2(3.2941176470588234) * direction;
    vec2 off3 = vec2(5.176470588235294) * direction;
    color += vec4(glowColor, texture2D(image, uv).a) * 0.1964825501511404;
    color += vec4(glowColor, texture2D(image, uv + (off1 / resolution)).a) * 0.2969069646728344;
    color += vec4(glowColor, texture2D(image, uv - (off1 / resolution)).a) * 0.2969069646728344;
    color += vec4(glowColor, texture2D(image, uv + (off2 / resolution)).a) * 0.09447039785044732;
    color += vec4(glowColor, texture2D(image, uv - (off2 / resolution)).a) * 0.09447039785044732;
    color += vec4(glowColor, texture2D(image, uv + (off3 / resolution)).a) * 0.010381362401148057;
    color += vec4(glowColor, texture2D(image, uv - (off3 / resolution)).a) * 0.010381362401148057;
    gl_FragColor = color;
}