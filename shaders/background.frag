//#version 460 core
in vec3 vDireccion;

layout (binding = 0) uniform samplerCube uCielo;
layout (binding = 1) uniform sampler2D uTexturaSuelo; // la textura del terreno

uniform float uAlturaSuelo; // altura del plano que continua el terreno
uniform float uMetrosTesela; // metros que cubre una tesela en el terreno

out vec4 FragColor;

void main()
{
    vec3 d = normalize(vDireccion);

    if (d.y < 0.0) {
        float t = (uOjo.y - uAlturaSuelo) / max(-d.y, 1.0e-4); // Rayo P(t) = uOjo + t d contra el plano y = uAlturaSuelo:
        vec3 P = uOjo + t * d; // uOjo.y + t d.y = uAlturaSuelo -> t = (uOjo.y - uAlturaSuelo) / (-d.y)

        Material m;
        m.kd = a_lineal(texture(uTexturaSuelo, P.xz / uMetrosTesela).rgb);
        m.ks = vec3(0.0);
        m.ns = 1.0;

        vec3 color = iluminar(P, vec3(0.0, 1.0, 0.0), m);
        color = aplicar_niebla(color, P);
        FragColor = vec4(a_srgb(color), 1.0);
    } else {
        vec3 cielo = a_lineal(texture(uCielo, vDireccion).rgb);
        // Mezcla suave hacia la niebla cerca del horizonte (d.y -> 0)
        float f_horizonte = exp(-d.y * 12.0);
        cielo = mix(cielo, uNieblaColor, f_horizonte);
        FragColor = vec4(a_srgb(cielo), 1.0);
    }
}