#version 330 core 

out vec4 FragColor; 

in vec3 normal; 
in vec3 pos;
in vec2 tex_coord;

uniform float near;
uniform float far;

float linearize_depth(float depth) {
		float z = depth * 2.0 - 1.0;
		return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
		float depth = linearize_depth(gl_FragCoord.z) / far;
		FragColor = vec4(vec3(depth), 1.0);
}
