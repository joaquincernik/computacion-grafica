//#version 460 core

in vec3 vPosMundo;
in vec3 vNormal;
in vec2 vTexCoord;

layout (binding = 0) uniform sampler2D uTextura; // unidad 0

out vec4 FragColor;

void main()
{
    vec3 N = normalize(vNormal); // interpolada: hay que volver a normalizar
    Material m;
    m.kd = texture(uTextura, vTexCoord).rgb; // en el Practico 08 era el color final
    m.ks = vec3(0.0); // sin brillo especular
    m.ns = 1.0;

    vec3 color = iluminar(vPosMundo, N, m);
    color = aplicar_niebla(color, vPosMundo);
    FragColor = vec4(color, 1.0);
    }