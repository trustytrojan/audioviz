// original: https://www.rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
#version 460 core

const float[] offset = float[](0, 1.3846153846, 3.2307692308);
const float[] weight = float[](0.2270270270, 0.3162162162, 0.0702702703);

uniform sampler2D image;

// (x, 0) = horizontal
// (0, y) = vertical
// (x, y) = up-diagonal
// (-x, y) or (x, -y) = down-diagonal
// x, y = blur strength/radius
uniform vec2 direction;

out vec4 color;

void main()
{
	const vec2 coord = gl_FragCoord.xy;
	const ivec2 size = textureSize(image, 0);

	// start with original color.
	color = texture(image, coord / size) * weight[0];

	// we need to divide `coord` by `size` because `texture` expects normalized coordinates.

	// then, using `weight[1]` and `weight[2]`,
	for (int i = 1; i <= 2; ++i)
	{
		// add the color of nearby pixels.
		color += texture(image, (coord + direction * offset[i]) / size) * weight[i];
		color += texture(image, (coord - direction * offset[i]) / size) * weight[i];
	}
}