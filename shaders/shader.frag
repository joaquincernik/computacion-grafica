#version 460 core

in vec3 vNormal;           // normal interpolada que manda el vertex shader
out vec4 FragColor;

uniform vec3 uColor;
uniform vec3 uLightDir;    // direccion HACIA la luz, normalizada, en coordenadas de mundo

void main()
{
    vec3 N = normalize(vNormal);                 // la interpolacion la desnormaliza
    float difusa = max(dot(N, uLightDir), 0.0);  // coseno del angulo entre normal y luz (Lambert)
    float ambiente = 0.25;                       // piso para que las caras en sombra no queden negras
    FragColor = vec4(uColor * (ambiente + (1.0 - ambiente) * difusa), 1.0);
}
