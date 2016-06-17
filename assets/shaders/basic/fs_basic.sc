$input v_texcoord0, v_color0

#include "../common.sh"

SAMPLER2D(s_texture, 0);

void main()
{
	vec4 color = texture2DLod(s_texture, v_texcoord0, 1);
	gl_FragColor = color;
}

