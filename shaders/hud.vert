#version 460 core

layout (location = 0) in vec3 vertexPos;

uniform mat4 uModel;
uniform mat4 uProjection;

void main()
{
    gl_Position = uProjection * uModel * vec4(vertexPos, 1.0);
}