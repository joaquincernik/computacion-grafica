//#version 460 core

in vec3 vPosMundo;
in vec3 vNormal;

layout (binding = 0) uniform samplerCube uCielo;

uniform Material uMaterial;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(vNormal);                 // la interpolacion la desnormaliza
    Material m = uMaterial;
    m.kd = a_lineal(uMaterial.kd);

    vec3 color = iluminar(vPosMundo, N, m);

    // Reflexión del cielo sobre la superficie
    vec3 V = normalize(uOjo - vPosMundo);
    vec3 R = reflect(-V, N);
    vec3 reflejoCielo = a_lineal(texture(uCielo, R).rgb) * uFactorCielo;

    // Usamos la componente especular (m.ks) para que solo brillen/reflejen las piezas pulidas
    float factorReflejo = clamp(m.ks.r * 0.45, 0.0, 0.6);
    color = mix(color, reflejoCielo, factorReflejo);

    color = aplicar_niebla(color, vPosMundo);

    FragColor = vec4(a_srgb(color), 1.0);
}