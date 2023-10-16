#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 FragColor;

uniform sampler2D SeaTex;
uniform sampler2D Block;

uniform int Tex;

uniform DirLight dirLight;
uniform vec3 viewPos;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir,sampler2D Map);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result=vec3(0.0f);

    if(Tex==0){
        result = CalcDirLight(dirLight,norm,viewDir,SeaTex);
        FragColor=vec4(result,1.0f);
	}
    else{
        result = CalcDirLight(dirLight,norm,viewDir,Block);
        FragColor=vec4(result,1.0f);
	}

} 


// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir,sampler2D Map)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(Map, TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(Map, TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(Map, TexCoord));
    return (ambient + diffuse + specular);
}