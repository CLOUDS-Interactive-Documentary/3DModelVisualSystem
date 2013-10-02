
#version 120
#extension GL_ARB_texture_rectangle : enable

uniform float specularScale = 1.;
uniform float specularExpo = 64.;

varying vec3 norm;
varying vec3 ePos;
varying vec2 uv;

void main(void)
{
	
	float fr = dot( -normalize(ePos), norm ) * .5 + .5;
	fr *= pow( specularScale * fr, specularExpo);
	gl_FragColor = vec4( norm * .5 + .5 + fr, 1.);
}

