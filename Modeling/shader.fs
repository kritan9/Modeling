#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
}; 


struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

#define NR_POINT_LIGHTS 2

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;
in vec3 MatAmbi;
in vec3 MatDiff;
in vec3 MatSpec;
in float Shini;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform sampler2D shadowMapD;
uniform samplerCube shadowMapP[NR_POINT_LIGHTS];
//uniform samplerCube shadowMapS;
uniform int diffuseN;
uniform int specularN;
uniform int nrComponents_d;
uniform int nrComponents_sp;
uniform bool spot;


uniform float far_plane;
vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir,int i);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float ShadowCalculationD(vec4 fragPosLightSpace,vec3 normal, vec3 lightDir);
float ShadowCalculationP(vec3 fragPos,vec3 lightPos, int i);


void main()
{   
	
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
	//vec4 col =vec4(texture(material.texture_diffuse1, TexCoords));
	//float w=col.w;
	
    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================
    // phase 1: directional lighting

   vec3 result = CalcDirLight(dirLight, norm, viewDir);
   //phase 2: point lights
   for(int i = 0; i < NR_POINT_LIGHTS; i++)
       result += CalcPointLight(pointLights[i], norm, FragPos, viewDir,i);    
     //phase 3: spot light
   if(spot) result += CalcSpotLight(spotLight, norm, FragPos, viewDir);    
    
    FragColor = vec4(result, 1.0);
	//else FragColor = vec4(result, w);
	
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir=normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), Shini);
    // combine results
	vec3 ambient = light.ambient;
	vec3 diffuse = light.diffuse * diff ;
	vec3 specular = light.specular * spec;
	
	if(diffuseN>0)
	{
		 ambient *=  vec3(texture(material.texture_diffuse1, TexCoords));
		 diffuse *=  vec3(texture(material.texture_diffuse1, TexCoords));
	}
	else{
		ambient*=MatAmbi;
		diffuse*=MatDiff;
	}
	if(specularN>0)
	specular *=  vec3(texture(material.texture_specular1, TexCoords));
	else specular*= MatSpec;

	// calculate shadow
    float shadow = ShadowCalculationD(FragPosLightSpace,normal, lightDir);  

	return (ambient + (1.0 - shadow) *(diffuse + specular));
   
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir,int i)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir=normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), Shini);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
   
   vec3 ambient = light.ambient;
	vec3 diffuse = light.diffuse * diff ;
	vec3 specular = light.specular * spec;
	
	if(diffuseN>0)
	{
		 ambient *=  vec3(texture(material.texture_diffuse1, TexCoords));
		 diffuse *=  vec3(texture(material.texture_diffuse1, TexCoords));
	}
	else{
		ambient*=MatAmbi;
		diffuse*=MatDiff;
	}
	if(specularN>0)
	specular *=  vec3(texture(material.texture_specular1, TexCoords));
	else specular*= MatSpec;

	ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
	// calculate shadow
    float shadow = ShadowCalculationP(fragPos,light.position,i);  

	return (ambient + (1.0 - shadow) *(diffuse + specular));
	
 
   
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), Shini);
	vec3 halfwayDir=normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), Shini);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results

	vec3 ambient = light.ambient;
	vec3 diffuse = light.diffuse * diff ;
	vec3 specular = light.specular * spec;
	
	if(diffuseN>0)
	{
		 ambient *=  vec3(texture(material.texture_diffuse1, TexCoords));
		 diffuse *=  vec3(texture(material.texture_diffuse1, TexCoords));
	}
	else{
		ambient*=MatAmbi;
		diffuse*=MatDiff;
	}
	if(specularN>0)
	specular *=  vec3(texture(material.texture_specular1, TexCoords));
	else specular*= MatSpec;

	ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    // calculate shadow
    float shadow = 0.0f;//ShadowCalculationP(fragPos,light.position,-1);  

	return (ambient + (1.0 - shadow) *(diffuse + specular));
	

    
}


float ShadowCalculationD(vec4 fragPosLightSpace,vec3 normal, vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMapD, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 
    float shadow = 0.0;
vec2 texelSize = 1.0 / textureSize(shadowMapD, 0);
for(int x = -1; x <= 1; ++x)
{
    for(int y = -1; y <= 1; ++y)
    {
        float pcfDepth = texture(shadowMapD, projCoords.xy + vec2(x, y) * texelSize).r; 
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
    }    
}
shadow /= 9.0;
	if(projCoords.z > 1.0) shadow = 0.0;

    return shadow;
}
float ShadowCalculationP(vec3 fragPos,vec3 lightPos, int i)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // ise the fragment to light vector to sample from the depth map 
	float closestDepth;
	//if(i==-1) closestDepth = texture(shadowMapS, fragToLight).r;
     closestDepth = texture(shadowMapP[i], fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    float shadow = 0.0;
float bias   = 0.15;
int samples  = 20;
float viewDistance = length(viewPos - fragPos);
float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;  
for(int j = 0; j < samples; ++j)
{
    //float closestDepth = (i==-1)? texture(shadowMapS, fragToLight + sampleOffsetDirections[j] * diskRadius).r:texture(shadowMapP[i], fragToLight + sampleOffsetDirections[j] * diskRadius).r;
	float closestDepth = texture(shadowMapP[i], fragToLight + sampleOffsetDirections[j] * diskRadius).r;
    closestDepth *= far_plane;   // Undo mapping [0;1]
    if(currentDepth - bias > closestDepth)
        shadow += 1.0;
}
shadow /= float(samples);  
        
    return shadow;
}