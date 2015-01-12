#version 150

in vec4		Color;
in vec3		Normal;
in vec2		TexCoord;

out vec4 	oColor;

void main(void) {
vec4 color = vec4(0., 0., 0., 0.);
vec4 diffuse = vec4(0., 0., 0., 1.);
color.xyz += diffuse.xyz;
color = vec4(color.rgb * diffuse.a, diffuse.a);
oColor = color;
}
