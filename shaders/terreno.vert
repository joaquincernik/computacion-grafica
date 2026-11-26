#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // todavia no la lee nadie
layout (location = 2) in vec2 aTexCoord;

uniform mat3 uMatNormal;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uTeselas; // cuantas veces se repite la textura en la malla

out vec3 vPosMundo;
out vec3 vNormal;
out vec2 vTexCoord;

void main()
{
    vec4 posMundo = uModel * vec4(aPos, 1.0);
    vPosMundo = posMundo.xyz;
    vNormal   = uMatNormal * aNormal;
    vTexCoord = uTeselas * aTexCoord; // [0,1] -> [0,uTeselas]
    gl_Position = uProjection * uView *posMundo;
}