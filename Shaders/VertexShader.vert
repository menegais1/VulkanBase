#version 450

vec3 vertexData[3] = vec3[](
vec3(0.0, -0.5, 0),
vec3(0.5, 0.5, 0),
vec3(-0.5, 0.5, 0)
);

void main(){
    gl_Position = vec4(vertexData[gl_VertexIndex], 1.0);
}