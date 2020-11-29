#version 450

layout(vertices = 3) out;

//In parameters.
layout (location = 0) in vec2 tessellationControlTextureCoordinate[];
layout (location = 1) in vec3 tessellationControlPosition[];

//Out parameters.
layout (location = 0) out vec2 tessellationEvaluationTextureCoordinate[];
layout (location = 1) out vec3 tessellationEvaluationPosition[];


layout(set = 2, binding = 0) uniform TessInfo{
    vec3 tessLevelOuter;
    float tessLevelInner;
} tessInfo;




void main() {

    tessellationEvaluationTextureCoordinate[gl_InvocationID] = tessellationControlTextureCoordinate[gl_InvocationID];
    tessellationEvaluationPosition[gl_InvocationID] = tessellationControlPosition[gl_InvocationID];
    //Calculate tht tessellation levels.
    if (gl_InvocationID == 0)
    {
        gl_TessLevelInner[0] = tessInfo.tessLevelInner;
        gl_TessLevelOuter[0] = tessInfo.tessLevelOuter[0];
        gl_TessLevelOuter[1] = tessInfo.tessLevelOuter[1];
        gl_TessLevelOuter[2] = tessInfo.tessLevelOuter[1];
    }
}
