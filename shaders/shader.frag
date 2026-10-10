#version 460 core

//in vec3 vNormal;  Recibe la normal interpolada del vertex shader
out vec4 FragColor;
uniform vec3 uColor;

void main()
{
    FragColor = vec4(uColor,1.0);
}