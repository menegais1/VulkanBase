#version 450

//Layout specification.
layout (triangles, equal_spacing, ccw) in;

//In parameters.
layout (location = 0) in vec2 inUV[];
layout (location = 1) in vec3 inPosition[];


//Out parameters.
layout (location = 0) out vec2 outUV;


layout(set = 1, binding = 0) uniform MVP{
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

void main()
{
    //Pass the values along to the fragment shader.
    outUV = gl_TessCoord.x * inUV[0] + gl_TessCoord.y * inUV[1] + gl_TessCoord.z * inUV[2];
    vec3 position = (gl_TessCoord.x * inPosition[0] + gl_TessCoord.y * inPosition[1] + gl_TessCoord.z * inPosition[2]);

    gl_Position = mvp.projection * mvp.view * mvp.model * vec4(position, 1.0f);
}