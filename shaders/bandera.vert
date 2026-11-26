#version 460 core
layout (location = 0) in vec3 vertexPos;
layout (location = 2) in vec2 vertexUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
out vec2 vUV;

void main()
{
    vUV = vertexUV;
    gl_Position = uProjection * uView * uModel * vec4(vertexPos, 1.0);
}