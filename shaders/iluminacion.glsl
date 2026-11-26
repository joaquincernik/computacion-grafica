// iluminacion.glsl - codigo comun a todos los fragment shaders de la escena.
// Sin #version: esa linea la agrega el codigo que arma el programa.
#version 460 core
uniform vec3 uSolDir; // direccion HACIA el sol, en mundo, normalizada
uniform vec3 uSolColor; // color e intensidad del sol
uniform vec3 uAmbiente; // luz ambiente
uniform vec3 uOjo; // posicion de la camara, en mundo
uniform vec3 uNieblaColor; // color del horizonte del cielo (Parte 4)
uniform float uVisibilidad; // distancia a la que la niebla llega al 63 % (Parte 4)

struct Material
{
vec3 kd; // coeficiente difuso (y ambiente: ka = kd)
vec3 ks; // coeficiente especular
float ns; // exponente especular (brillo)
};

// Modelo de Phong con una luz direccional. P y N en coordenadas de mundo, N normalizada.
vec3 iluminar(vec3 P, vec3 N, Material m)
{
vec3 L = uSolDir; // direccional: la misma en todo punto
vec3 V = normalize(uOjo - P); // hacia el ojo
float NdotL = max(dot(N, L), 0.0);
vec3 ambiente = uAmbiente * m.kd;
vec3 difusa = uSolColor * m.kd * NdotL;
vec3 especular = vec3(0.0);

if (NdotL > 0.0) {
vec3 R = reflect(-L, N);
especular = uSolColor * m.ks * pow(max(dot(R, V), 0.0), m.ns);
}
return ambiente + difusa + especular;
}
// Niebla exponencial: mezcla el color con el de la niebla segun la distancia al ojo.
vec3 aplicar_niebla(vec3 color, vec3 P)
{
float d = length(P - uOjo);
float f = 1.0 - exp(-d / uVisibilidad);
return mix(color, uNieblaColor, f);
}