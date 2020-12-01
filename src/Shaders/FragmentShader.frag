#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inPos;


layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D u_texture;
layout(set = 0, binding = 1) uniform LightInformation{
    vec3 position;
    vec3 cameraPosition;
}lightInformation;


void main(){
    float diffuse = dot(normalize(inNormal),normalize(lightInformation.position - inPos));
    if(diffuse < 0) diffuse = 0.1;
    fragColor = vec4(1,1,1,1) * diffuse;
}