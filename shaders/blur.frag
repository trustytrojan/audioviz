// original: https://www.rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
#version 120

const float[] offset = float[](0, 1.3846153846, 3.2307692308);
const float[] weight = float[](0.2270270270, 0.3162162162, 0.0702702703);

// in 1.10, arrays can't be constructed in-shader.
// they must be passed as uniforms...
// uniform float offset[3];
// uniform float weight[3];

uniform sampler2D image;
uniform vec2 size; // in <1.30 we have to pass the size manually

// (x, 0) = horizontal
// (0, y) = vertical
// (x, y) = up-diagonal
// (-x, y) or (x, -y) = down-diagonal
// x, y = blur strength/radius
uniform vec2 direction;

void main()
{
	// normalize coordinates
	vec2 coord = gl_FragCoord.xy;

	// start with the original color, apply first weight.
	gl_FragColor = texture2D(image, coord / size) * weight[0];

	// then, using `weight[1]` and `weight[2]`,
	for (int i = 1; i <= 2; ++i)
	{
		// add the color of nearby pixels.
		gl_FragColor += texture2D(image, (coord + direction * offset[i]) / size) * weight[i];
		gl_FragColor += texture2D(image, (coord - direction * offset[i]) / size) * weight[i];
	}
}