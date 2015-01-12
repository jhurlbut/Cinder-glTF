#version 150

in vec4		Color;
in vec3		Normal;
in vec2		TexCoord;

out vec4 	oColor;


uniform vec4 u_diffuse;
uniform vec4 u_emission;
uniform vec4 u_ambient;

void main(void) {
vec3 normal = normalize( Normal );
if (gl_FrontFacing == false) normal = -normal;
vec4 color = vec4(0., 0., 0., 0.);
vec4 diffuse = vec4(0., 0., 0., 1.);
vec4 emission;
vec4 ambient;
ambient = u_ambient;
diffuse = u_diffuse;
emission = u_emission;
diffuse.xyz *= max(dot(normal,vec3(0.,0.,1.)), 0.);
color.xyz += diffuse.xyz;
color.xyz += emission.xyz;
color = vec4(color.rgb * diffuse.a, diffuse.a);
	oColor = color;
}
