#version 460 core
in vec3 vDireccion;
layout (binding = 0) uniform samplerCube uCielo;
layout (binding = 1) uniform sampler2D uTexturaSuelo; // la textura del terreno
uniform vec3 uOjo; // posicion del observador en el mundo
uniform float uAlturaSuelo; // altura del plano que continua el terreno
uniform float uMetrosTesela; // metros que cubre una tesela en el terreno
out vec4 FragColor;
void main()
{
vec3 d = normalize(vDireccion);
// Rayo P(t) = uOjo + t d contra el plano y = uAlturaSuelo:
// uOjo.y + t d.y = uAlturaSuelo -> t = (uOjo.y - uAlturaSuelo) / (-d.y)
float t = (uOjo.y - uAlturaSuelo) / max(-d.y, 1.0e-4);
vec2 xz = uOjo.xz + t * d.xz; // punto del terreno visto
vec4 suelo = texture(uTexturaSuelo, xz / uMetrosTesela); // misma escala que la malla
vec4 cielo = texture(uCielo, vDireccion);
// Por debajo del horizonte el color es el terreno continuado; por encima, el cielo.
FragColor = (d.y < 0.0) ? suelo : cielo;
}