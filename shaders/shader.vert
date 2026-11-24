#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // Atributo de la normal que viene de la malla

out vec3 fColor; // Color iluminado que se interpolará entre vértices

uniform mat4 uModel;
uniform mat3 uMatNormal;
uniform mat4 uView;
uniform mat4 uProjection;

// Material
uniform vec3 uColor;
uniform vec3 uKs;       
uniform float uBrillo;  // Exponente especular ns (ej: 32.0)

// Luz y cámara (en coordenadas de mundo)
uniform vec3 uLuzPos;   // Posición de la luz (o dirección si es el sol)
uniform vec3 uLuzColor; // Ej: vec3(1.0, 1.0, 1.0)
uniform vec3 uAmbiente; // Ej: vec3(0.2, 0.2, 0.2)
uniform vec3 uOjo;      // Posición de la cámara en mundo

vec3 phong(vec3 P, vec3 N)
{
    // Para luz puntual: normalize(uLuzPos - P)
    // Para luz direccional (sol): normalize(uLuzPos)
    vec3 L = normalize(uLuzPos - P);
    vec3 V = normalize(uOjo - P);

    float NdotL = max(dot(N, L), 0.0);

    vec3 ambiente = uAmbiente * uColor;
    vec3 difusa   = uLuzColor * uColor * NdotL;
    vec3 especular = vec3(0.0);

    if (NdotL > 0.0) {
        vec3 R = reflect(-L, N);
        especular = uLuzColor * uKs * pow(max(dot(R, V), 0.0), uBrillo);
    }

    return ambiente + difusa + especular;
}

void main()
{
    vec4 pm = uModel * vec4(aPos, 1.0);
    vec3 N = normalize(uMatNormal * aNormal);

    fColor = phong(pm.xyz, N);

    gl_Position = uProjection * uView * pm;

    
}