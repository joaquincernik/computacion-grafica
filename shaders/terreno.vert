#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // todavia no la lee nadie
layout (location = 2) in vec2 aTexCoord;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uTeselas; // cuantas veces se repite la textura en la malla
out vec2 vTexCoord;
void main()
{
    vTexCoord = uTeselas * aTexCoord; // [0,1] -> [0,uTeselas]
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}