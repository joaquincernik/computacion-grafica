#version 460 core
in vec2 vTexCoord;
layout (binding = 0) uniform sampler2D uTextura; // unidad 0
out vec4 FragColor;
void main()
{
    FragColor = texture(uTextura, vTexCoord); // REEMPLAZA el color
}