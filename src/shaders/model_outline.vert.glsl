#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; 
layout (location = 2) in vec2 aTexCoord; 

out vec3 pos;
out vec3 normal; 
out vec2 tex_coord;

uniform mat4 model; 
uniform mat4 view; 
uniform mat4 projection; 

uniform mat4 outline_scale; 

void main()
{
  mat4 real_model = outline_scale * model;
  gl_Position = projection * view * real_model * vec4(aPos, 1.0f);
  normal = aNormal; 
  tex_coord = aTexCoord;
  pos = vec3(real_model*vec4(aPos,1));
}
