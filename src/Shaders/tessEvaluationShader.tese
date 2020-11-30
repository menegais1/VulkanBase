#version 450

//Layout specification.
layout (triangles, equal_spacing, ccw) in;

//In parameters.
layout (location = 0) in vec2 inUV[];
layout (location = 1) in vec3 inPosition[];

layout(set = 0, binding = 0) uniform sampler2D uniform_heightmap;

//Out parameters.
layout (location = 0) out vec2 outUV;

void main()
{
    //Pass the values along to the fragment shader.
    outUV = gl_TessCoord.x * inUV[0] + gl_TessCoord.y * inUV[1] + gl_TessCoord.z * inUV[2];
    vec3 position = (gl_TessCoord.x * inPosition[0] + gl_TessCoord.y * inPosition[1] + gl_TessCoord.z * inPosition[2]);
    position.y = position.y + texture(uniform_heightmap, outUV).r;
    gl_Position =  vec4(position.x, position.y,position.z, 1.0f);
}