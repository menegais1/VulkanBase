#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout (location = 0) in vec2 inUV[3];

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outPos;

layout(set = 1, binding = 0) uniform MVP{
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

void main(void)
{
    vec3 a = gl_in[0].gl_Position.xyz;
    vec3 b = gl_in[1].gl_Position.xyz;
    vec3 c = gl_in[2].gl_Position.xyz;

    vec3 dir = cross(b - a, c - a);
    outNormal = normalize(dir);
    gl_Position = mvp.projection * mvp.view* mvp.model * vec4(a, 1.0);
    outPos = (mvp.model * vec4(a, 1.0)).xyz;
    outUV = inUV[0];
    EmitVertex();

    outNormal = normalize(dir);
    gl_Position = mvp.projection * mvp.view* mvp.model * vec4(b, 1.0);
    outPos = (mvp.model * vec4(b, 1.0)).xyz;
    outUV = inUV[1];
    EmitVertex();

    outNormal = normalize(dir);
    gl_Position = mvp.projection * mvp.view* mvp.model * vec4(c, 1.0);
    outPos = (mvp.model * vec4(c, 1.0)).xyz;
    outUV = inUV[2];
    EmitVertex();

    EndPrimitive();

}