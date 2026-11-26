//#version 460 core

in vec3 vPosMundo;
in vec3 vNormal;

uniform Material uMaterial;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(vNormal);                 // la interpolacion la desnormaliza
    Material m = uMaterial;
    m.kd = a_lineal(uMaterial.kd);
    
    vec3 color = iluminar(vPosMundo, N, m);
    color = aplicar_niebla(color, vPosMundo);

    FragColor = vec4(a_srgb(color), 1.0);
}
