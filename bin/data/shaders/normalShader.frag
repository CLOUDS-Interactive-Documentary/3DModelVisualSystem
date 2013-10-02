varying vec3 norm;
varying vec2 uv;

void main(void)
{
	gl_FragColor = vec4( norm, 1.); // vec4( uv/100., 1., 1.);//
}

