#version 150

in vec4		Color;
in vec3		Normal;
in vec2		TexCoord;

out vec4 	oColor;


uniform sampler2D u_diffuse;
uniform vec4 u_emission;
uniform vec4 u_specular;
uniform float u_shininess;
in vec3 v_light0Direction;
uniform vec3 u_light0Color;
uniform vec4 u_ambient;

void main(void) {
	vec3 normal = normalize( -Normal );
	vec4 color = vec4(0., 0., 0., 0.);
	vec4 diffuse = vec4(0., 0., 0., 1.);
	vec3 diffuseLight = vec3(0., 0., 0.);
	vec4 emission;
	vec4 ambient;
	vec4 specular;
	ambient = u_ambient;

	diffuse = texture(u_diffuse, vec2(TexCoord.s,-TexCoord.t));
	emission = u_emission;
	specular = u_specular;
	vec3 specularLight = vec3(0., 0., 0.);
	{
		float specularIntensity = 0.;
		float attenuation = 1.0;
		vec3 l = normalize(v_light0Direction);
		vec3 h = normalize(l+vec3(0.,0.,1.));
		specularIntensity = max(0., pow(max(dot(normal,h), 0.) , u_shininess)) * attenuation;
		specularLight += u_light0Color * specularIntensity;
		diffuseLight += u_light0Color * max(dot(normal,l), 0.) * attenuation;
	}
	specular.xyz *= specularLight;
	color.xyz += specular.xyz;
	diffuse.xyz *= diffuseLight;
	color.xyz += diffuse.xyz;
	color.xyz += emission.xyz + ambient.xyz;
	color = vec4(color.rgb * diffuse.a, diffuse.a);
	oColor = color;
	//oColor = vec4(TexCoord.s,0,0,1);
	//vec3 l = normalize(v_light0Direction);
	//vec3 test = u_light0Color * max(dot(normal,l), 0.);
	//oColor.rgb = test;
}
