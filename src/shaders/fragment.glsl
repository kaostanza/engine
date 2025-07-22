#version 330 core
out vec4 FragColor; 

in vec3 ourColor; 
in vec3 pos;
in vec2 TexCoord;

// in vec2 TexCoord;
// uniform sampler2D texture1;
// uniform sampler2D texture2;
// uniform float mix_val; 

uniform vec3 viewPos;

struct Light {
  vec3 position; 

  vec3 ambient;
  vec3 diffuse;
  vec3 specular; 
};

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
};

struct Material {
  sampler2D diffuse; 
  sampler2D specular;
  sampler2D emission;
  float shininess;
};

uniform SpotLight light;
uniform Material material; 

void main()
{
 //FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
 // FragColor = vec4(ourColor, 1.0f); 
 //FragColor = mix(texture(texture1, TexCoord),texture(texture2, vec2(TexCoord.x,-TexCoord.y)), mix_val); 

  // PointLight
  // float dist = length(pos-light.position);
  // float attenuation = 1.0 / (light.constant + light.linear*dist + light.quadratic*pow(dist,2));
  // vec3 direction = normalize(pos-light.position);

  // SpotLight
  vec3 lightDir = normalize(light.position - pos);
  float theta = dot(lightDir, normalize(-light.direction));
  float epsilon = light.inner_cut_off - light.outer_cut_off;
  float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);


  vec3 ambient = vec3(texture(material.diffuse, TexCoord))* light.ambient; 

  vec3 emission = vec3(texture(material.emission, TexCoord));

  // diffuse
  vec3 norm = normalize(cross(dFdx(pos),dFdy(pos)));
  float diffusion = max(dot(lightDir,norm), 0.0);
  vec3 diffuse = vec3(texture(material.diffuse, TexCoord))* diffusion * light.diffuse;


  // specular
  vec3 viewDir = normalize(viewPos - pos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir),0.0), material.shininess);
  vec3 specular = vec3(texture(material.specular, TexCoord)) * spec * light.specular;

// ambient *= attenuation; 
  // specular*= attenuation; 
  // diffuse*= attenuation; 

  diffuse *= intensity;
  specular *= intensity;


  FragColor = vec4((ambient + diffuse + specular + emission) , 1.0f);






  }

