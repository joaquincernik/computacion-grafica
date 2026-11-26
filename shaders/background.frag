in vec3 vDireccion;

layout (binding = 0) uniform samplerCube uCielo;
layout (binding = 1) uniform sampler2D uTexturaSuelo; // la textura del terreno


uniform float uAlturaSuelo; // altura del plano que continua el terreno
uniform float uMetrosTesela; // metros que cubre una tesela en el terreno

out vec4 FragColor;

void main()
{
    vec3 d = normalize(vDireccion);
    vec3 color;

    if (d.y < 0.0) {
        float t = (uOjo.y - uAlturaSuelo) / max(-d.y, 1.0e-4);
        vec3 P = uOjo + t * d;

        Material m;
        m.kd = a_lineal(texture(uTexturaSuelo, P.xz / uMetrosTesela).rgb);
        m.ks = vec3(0.0);
        m.ns = 1.0;

        color = iluminar(P, vec3(0.0, 1.0, 0.0), m);
        color = aplicar_niebla(color, P);
    } else {
        // El rayo apunta hacia el horizonte o hacia arriba -> cielo
        vec3 cielo = a_lineal(texture(uCielo, d).rgb)* uFactorCielo;
        
        // Fundir los primeros ~5.7 grados de elevación con el color de la niebla
        float f = 1.0 - smoothstep(0.0, 0.10, d.y);
        cielo = mix(cielo, a_lineal(uNieblaColor), f);
        
        color = cielo;
    }

    FragColor = vec4(a_srgb(color), 1.0);
}