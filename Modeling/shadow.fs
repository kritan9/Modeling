#version 330 core
struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
}; 
//in vec2 TexCoords;
//uniform int nrComponents_d;
//uniform Material material;
//uniform int diffuseN;
void main()
{             
    // gl_FragDepth = gl_FragCoord.z;
	//if(diffuseN!=0){if(nrComponents_d==4){if(vec4(texture(material.texture_diffuse1,TexCoords)).w<0.1) discard;}}
}