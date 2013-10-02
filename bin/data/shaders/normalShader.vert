varying vec3 norm;
varying vec3 ePos;
varying vec2 uv;

void main()
{
	norm = normalize( gl_NormalMatrix * gl_Normal );
	norm = norm * .5 + .5;
	
	vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;
	gl_Position = gl_ProjectionMatrix * ecPosition;
	
	uv = gl_MultiTexCoord0.xy;
}

