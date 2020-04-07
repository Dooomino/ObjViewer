/*
 *  Simple fragment sharder for Lab 2
 */

#version 330 core
in vec3 normal;
in vec3 position;
in vec2 tc;
uniform sampler2D texa;
uniform mat4 modelView;
uniform vec3 Eye;
uniform vec4 material;
void main() {
	vec3 lightPos = vec3(5.0,10.0,5.0);

	vec4 light_colour = vec4(1.0,1,0.0,1.0);
	vec3 N=normalize(normal);
	vec4 base = vec4(1.0,0.,0.,1.0);
	vec4 ambient = light_colour;


	vec4 white = vec4(1.0, 1.0, 1.0, 1.0);

	float diffuse;


	vec3 L = normalize(lightPos - position);

	vec3 V = normalize(Eye - position);

	vec3 R = normalize(reflect(-L,N));

	vec4 text;
	float specular;

	diffuse = max(dot(N,L),0.0);

	specular = pow(max(0.0, dot(V,R)),material.w);

	text = texture(texa, tc);

	if(	text.x == 0 && 
		text.y == 0 && 
		text.z == 0
		){

		gl_FragColor = min((material.x*ambient + 
							material.y*diffuse + 
							material.z*light_colour*specular
							)*base, vec4(1.0));

	}else{
		gl_FragColor = min(material.x*text + material.y*diffuse*text + material.z*white*specular, vec4(1.0));
	}


	gl_FragColor.a = 1.0;
}



