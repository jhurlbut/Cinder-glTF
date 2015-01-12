#version 150

in vec4		Color;
in vec3		Normal;
in vec2		TexCoord;

out vec4 	oColor;

uniform vec4 u_diffuse;
uniform float u_transparency;
void main(void) {
vec3 normal = normalize( Normal );
vec4 color = vec4(0., 0., 0., 0.);
vec4 diffuse = vec4(0., 0., 0., 1.);
diffuse = u_diffuse;
diffuse.xyz *= max(dot(normal,vec3(0.,0.,1.)), 0.);
color.xyz += diffuse.xyz;
color = vec4(color.rgb * diffuse.a, diffuse.a * u_transparency);
oColor = color;
}
