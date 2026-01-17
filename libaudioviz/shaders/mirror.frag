#version 110

uniform sampler2D image;
uniform vec2 size;
uniform int mirror_side; // 0 = mirror right, 1 = mirror left

void main()
{
	vec2 uv = gl_FragCoord.xy / size;
	
	if (mirror_side == 0)
	{
		// Mirror right side: if we're on the right half, mirror to left
		if (uv.x > 0.5)
		{
			uv.x = 1.0 - uv.x;
		}
	}
	else if (mirror_side == 1)
	{
		// Mirror left side: if we're on the left half, mirror to right
		if (uv.x < 0.5)
		{
			uv.x = 1.0 - uv.x;
		}
	}
	
	gl_FragColor = texture2D(image, uv);
}
