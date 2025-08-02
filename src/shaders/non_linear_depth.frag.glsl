#version 330 core 

out vec4 FragColor; 

in vec3 normal; 
in vec3 pos;
in vec2 tex_coord;

void main() {
		FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}
