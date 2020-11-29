#version 450

//Layout specification.
layout (triangles, equal_spacing, ccw) in;

//In parameters.
layout (location = 0) in vec2 tessellationEvaluationTextureCoordinate[];
layout (location = 1) in vec3 tessellationEvaluationPosition[];


//Out parameters.
layout (location = 1) out vec2 fragmentTextureCoordinate;


layout(set = 1, binding = 0) uniform MVP{
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

void main()
{
    //Pass the values along to the fragment shader.
    fragmentTextureCoordinate = gl_TessCoord.x * tessellationEvaluationTextureCoordinate[0] + gl_TessCoord.y * tessellationEvaluationTextureCoordinate[1] + gl_TessCoord.z * tessellationEvaluationTextureCoordinate[2];
    vec3 position = (gl_TessCoord.x * tessellationEvaluationPosition[0] + gl_TessCoord.y * tessellationEvaluationPosition[1] + gl_TessCoord.z * tessellationEvaluationPosition[2]);

    gl_Position = mvp.projection * mvp.view * mvp.model * vec4(position, 1.0f);
}