#version 460 core
layout (location = 0) in vec3 aPos;
uniform mat4 uProjection;
uniform mat4 uView; // la vista sin traslacion
out vec3 vDireccion;
void main()
{
    vDireccion = aPos; // desde el centro, cada vertice es una direccion
    gl_Position = uProjection * uView * vec4(aPos, 1.0);
}