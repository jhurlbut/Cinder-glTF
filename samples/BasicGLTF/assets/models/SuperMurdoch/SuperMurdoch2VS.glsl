#version 150

uniform mat4	ciModelViewProjection;
uniform mat3	ciNormalMatrix;


in vec4		ciPosition;
in vec3		ciNormal;
in vec4		ciColor;

out lowp vec4	Color;
out highp vec3	Normal;

void main(void) {
	vec4 pos = ciPosition;
	Normal		= ciNormalMatrix * ciNormal;
	
	gl_Position	= ciModelViewProjection * ciPosition;
}
