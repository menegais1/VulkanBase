#version 450
layout(location = 0) in vec2 i_texCoord;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D u_texture;


void main(){
    fragColor = texture(u_texture, i_texCoord);
}