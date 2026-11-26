#pragma once

#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/constants.hpp>

struct EntornoSolar
{
    glm::vec3 sol_dir;
    glm::vec3 sol_color;
    glm::vec3 ambiente;
    glm::vec3 niebla_color;
    float factor_cielo;
};

inline EntornoSolar calcular_entorno_solar(double tiempo)
{
    // Ciclo completo cada 60 segundos (ajustable para probar rápido, ej: 20.0f)
    const float duracion_dia = 60.0f;
    float angulo = static_cast<float>(tiempo) * (glm::two_pi<float>() / duracion_dia);

    // El sol gira en el plano XY (sale en +X, cenit en +Y, se pone en -X)
    // Le damos un leve desplazamiento en Z (-0.3f) para que ilumine con ángulo 3D
    glm::vec3 dir = glm::normalize(glm::vec3(glm::cos(angulo), glm::sin(angulo), -0.3f));
    float elevacion = dir.y; // seno de la altura sobre el horizonte [-1, 1]

    // 1. Apagar el sol cuando está por debajo del horizonte (con transición suave entre -0.05 y 0.05)
    float sol_activo = glm::smoothstep(-0.05f, 0.05f, elevacion);

    // 2. Color del sol: blanco cálido arriba, anaranjado cerca del horizonte
    float t_ocaso = 1.0f - glm::smoothstep(0.0f, 0.35f, glm::max(elevacion, 0.0f));
    glm::vec3 color_mediodia(0.85f, 0.82f, 0.78f);
    glm::vec3 color_horizonte(1.00f, 0.42f, 0.12f);
    glm::vec3 sol_color = glm::mix(color_mediodia, color_horizonte, t_ocaso) * sol_activo;

    // 3. Factor de día para el cielo y la niebla (de 0.08 en plena noche a 1.0 de día)
    float factor_dia = glm::mix(0.08f, 1.0f, glm::smoothstep(-0.15f, 0.25f, elevacion));

    // 4. Luz ambiente: azulada tenue de noche, neutra de día
    glm::vec3 amb_noche(0.03f, 0.04f, 0.07f);
    glm::vec3 amb_dia(0.22f, 0.22f, 0.25f);
    glm::vec3 ambiente = glm::mix(amb_noche, amb_dia, factor_dia);

    // 5. Color de la niebla (en sRGB, porque los shaders lo pasan por a_lineal):
    // Se oscurece de noche y toma un tinte cálido en el amanecer/atardecer
    glm::vec3 niebla_dia(0.55f, 0.68f, 0.85f);
    glm::vec3 niebla_ocaso(0.75f, 0.45f, 0.35f);
    float banda_ocaso = glm::smoothstep(-0.15f, 0.05f, elevacion) * (1.0f - glm::smoothstep(0.05f, 0.35f, elevacion));
    glm::vec3 niebla_base = glm::mix(niebla_dia, niebla_ocaso, banda_ocaso * 0.7f);
    glm::vec3 niebla_color = niebla_base * factor_dia;

    return {dir, sol_color, ambiente, niebla_color, factor_dia};
}