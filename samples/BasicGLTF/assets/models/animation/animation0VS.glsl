#version 150

uniform mat4	ciModelViewProjection;
uniform mat3	ciNormalMatrix;


uniform mat4 u_light0Transform;

in vec4		ciPosition;
in vec2		ciTexCoord0;
in vec3		ciNormal;
in vec4		ciColor;

out highp vec2	TexCoord;
out lowp vec4	Color;
out highp vec3	Normal;
out highp vec3 v_light0Direction;

void main(void) {
	vec4 pos = ciPosition;
	Normal		= ciNormalMatrix * ciNormal;
	TexCoord	= ciTexCoord0;
	v_light0Direction = mat3(u_light0Transform) * vec3(0.,0.,1.);
	gl_Position	= ciModelViewProjection * ciPosition;
}
