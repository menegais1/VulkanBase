#version 450

layout(vertices = 3) out;

//In parameters.
layout (location = 0) in vec2 inUV[];
layout (location = 1) in vec3 inPosition[];

//Out parameters.
layout (location = 0) out vec2 outUV[];
layout (location = 1) out vec3 outPosition[];


layout(set = 2, binding = 0) uniform TessInfo{
    vec3 tessLevelOuter;
    float tessLevelInner;
} tessInfo;


void main() {

    outUV[gl_InvocationID] = inUV[gl_InvocationID];
    outPosition[gl_InvocationID] = inPosition[gl_InvocationID];
    //Calculate tht tessellation levels.
    if (gl_InvocationID == 0)
    {
        gl_TessLevelInner[0] = tessInfo.tessLevelInner;
        gl_TessLevelOuter[0] = tessInfo.tessLevelOuter[0];
        gl_TessLevelOuter[1] = tessInfo.tessLevelOuter[1];
        gl_TessLevelOuter[2] = tessInfo.tessLevelOuter[1];
    }
}
