#version 330 core
out vec4 FragColor; 

in vec3 normal; 
in vec3 pos;
in vec2 tex_coord;


struct DirectionalLight {
  vec3 direction; 
  
  vec3 ambient;
  vec3 diffuse;
  vec3 specular; 
};

struct PointLight {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular; 

  float constant;
  float linear;
  float quadratic;
};

struct SpotLight {
  vec3 position;
  vec3 direction; 
  float inner_cut_off;
  float outer_cut_off;

  vec3 ambient;
  vec3 specular;
  vec3 diffuse;

  float constant;
  float linear;
  float quadratic;
};

struct Material {
  sampler2D diffuse; 
  sampler2D specular;
  sampler2D emission;
  float shininess;
};

#define MAX_POINT_LIGHTS 4

uniform DirectionalLight directionnal_light;
uniform SpotLight spot_light;
uniform PointLight point_lights[MAX_POINT_LIGHTS];

uniform Material material; 
uniform vec3 view_pos;

vec3 process_spot_light(SpotLight light, vec3 position, vec3 view_position, vec3 normal);
vec3 process_directionnal_light(DirectionalLight light, vec3 position, vec3 view_position, vec3 normal);
vec3 process_point_light(PointLight light, vec3 position, vec3 view_position, vec3 normal);

void main()
{
  vec3 output = vec3(0.0);
  vec3 emission = vec3(texture(material.emission, tex_coord));

  output += process_directionnal_light(directionnal_light, pos, view_pos, normal);

  for (int i=0; i<MAX_POINT_LIGHTS; i++) {
    output += process_point_light(point_lights[i], pos, view_pos, normal);
  }

  output += process_spot_light(spot_light, pos, view_pos, normal);

  FragColor = vec4(output + emission , 1.0f);
}

vec3 process_point_light(PointLight light, vec3 position, vec3 view_position, vec3 normal) {
  vec3 light_dir = normalize(position - light.position);
  vec3 view_dir  = normalize(position - view_position);
  vec3 reflect_dir = reflect(-light_dir, normal);

  // specular
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = vec3(texture(material.specular, tex_coord)) * spec * light.specular;

  // diffuse
  float diffusion = max(dot(light_dir, normal), 0.0);
  vec3 diffuse = vec3(texture(material.diffuse, tex_coord)) * diffusion * light.diffuse;

  // attenuation
  float dist = length(position-light.position);
  float attenuation = 1.0 / (light.constant + light.linear*dist + light.quadratic*pow(dist,2));

  // ambient
  vec3 ambient = vec3(texture(material.diffuse , tex_coord)) * light.ambient;

  return (ambient + diffuse + specular) * attenuation;
}

vec3 process_directionnal_light(DirectionalLight light, vec3 position, vec3 view_position, vec3 normal) {
  vec3 light_dir = normalize(position - light.direction);
  vec3 view_dir  = normalize(position - view_position);
  vec3 reflect_dir = reflect(-light_dir, normal);

  // specular
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = vec3(texture(material.specular, tex_coord)) * spec * light.specular;

  // diffuse
  float diffusion = max(dot(light_dir, normal), 0.0);
  vec3 diffuse = vec3(texture(material.diffuse, tex_coord)) * diffusion * light.diffuse;

  // ambient
  vec3 ambient = vec3(texture(material.diffuse , tex_coord)) * light.ambient;

  return (ambient + diffuse + specular);
}

vec3 process_spot_light(SpotLight light, vec3 position, vec3 view_position, vec3 normal) {
  vec3 light_dir = normalize(light.position - position);
  vec3 view_dir  = normalize(position - view_position);
  vec3 reflect_dir = reflect(-light_dir, normal);

  // stuff
  float theta = dot(light_dir, normalize(-light.direction));
  float epsilon = light.inner_cut_off - light.outer_cut_off;
  float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);

  // specular
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = vec3(texture(material.specular, tex_coord)) * spec * light.specular;

  // diffuse
  float diffusion = max(dot(light_dir, normal), 0.0);
  vec3 diffuse = vec3(texture(material.diffuse, tex_coord)) * diffusion * light.diffuse;

  // attenuation
  float dist = length(position-light.position);
  float attenuation = 1.0 / (light.constant + light.linear*dist + light.quadratic*pow(dist,2));

  // ambient
  vec3 ambient = vec3(texture(material.diffuse , tex_coord)) * light.ambient;

  return (ambient + intensity * (diffuse + specular)) * attenuation;
}




