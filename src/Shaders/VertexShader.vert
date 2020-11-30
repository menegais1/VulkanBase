#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
//
//In parameters.
layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outPosition;

void main(){
    outPosition = inPosition;
    outUV = inUV;
}