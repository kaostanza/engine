#version 330 core 

out vec4 FragColor; 

in vec3 normal; 
in vec3 pos;
in vec2 tex_coord;

uniform vec3 outline_color;

void main() {
		FragColor = vec4(outline_color, 1.0);
}
