#version 450
layout(location = 0) in vec4 i_color;
layout(location = 1) in vec2 i_texCoord;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D u_texture;
layout(set = 0, binding = 1) uniform testBuffer{
    float s;
    float t;
} myTestBuffer;

void main(){
    fragColor = texture(u_texture, i_texCoord + vec2(myTestBuffer.s,myTestBuffer.t));
}