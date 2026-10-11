#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // Atributo de la normal que viene de la malla

out vec3 vNormal; // Variable de salida hacia el fragment shader

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    // Transformamos la normal con la matriz de rotación/escala del modelo
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;

    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}