#version 450

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_color;
layout(location = 2) in vec2 i_texCoord;

layout(location = 0) out vec4 o_color;
layout(location = 1) out vec2 o_texCoord;

void main(){
    gl_Position = vec4(i_position, 1.0);
    o_color = vec4(i_color, 1.0);
    o_texCoord = i_texCoord;
}