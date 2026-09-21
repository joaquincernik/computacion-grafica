#version 460 core

in vec3 vNormal; // Recibe la normal interpolada del vertex shader
out vec4 FragColor;

void main()
{
    // Normalizar para corregir imprecisiones de la interpolación
    vec3 N = normalize(vNormal);

    // Mapear el rango de normales [-1, 1] al rango de color RGB [0, 1]
    FragColor = vec4(N * 0.5 + 0.5, 1.0);
}