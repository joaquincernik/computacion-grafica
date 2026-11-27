//#version 460 core

in vec3 vPosMundo;
in vec3 vNormal;
in vec2 vTexCoord;

layout (binding = 0) uniform sampler2D uTextura; // unidad 0
uniform Material uMaterial; // kd lo pone la textura; ks y ns vienen del material   <-- NUEVO

out vec4 FragColor;

void main()
{
    vec3 N = normalize(vNormal); // interpolada: hay que volver a normalizar
    Material m;
    m.kd = a_lineal(texture(uTextura, vTexCoord).rgb); // la textura esta en sRGB
    m.ks = uMaterial.ks; //   <-- CAMBIA (antes: vec3(0.0))
    m.ns = uMaterial.ns; //   <-- CAMBIA (antes: 1.0)

    vec3 color = iluminar(vPosMundo, N, m);
    color = aplicar_niebla(color, vPosMundo);
    FragColor = vec4(a_srgb(color), 1.0);
}