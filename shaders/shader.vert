#version 460 core

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNormal; // Atributo de la normal que viene de la malla

out vec3 vPosMundo;
out vec3 vNormal;

uniform mat4 uModel;
uniform mat3 uMatNormal;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    vec4 wc_point = uModel * vec4(vertexPos, 1.0);
    vPosMundo = wc_point.xyz;
    vNormal   = uMatNormal * vertexNormal;
    gl_Position = uProjection * uView * wc_point;

    
}