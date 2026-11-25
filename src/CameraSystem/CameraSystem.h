#pragma once
#include "CameraSystem/CameraData.h"
#include "CameraSystem/CameraCommand.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CameraSystem
{
public:
    CameraSystem(int width, int height); // arma la proyeccion inicial
    void update(glm::vec3 pos_avion,     // donde esta el objetivo
                glm::vec3 angulos_avion, // no se usa por ahora (ver Parte 3)
                const CameraCommand &cmd,
                float dt);                    // se actualiza en cada cuadro
    void set_viewport(int width, int height); // al redimensionar la ventana
    const CameraData &data() const;           // los datos para construir la proyección
private:
    float yaw_ = 0.0f, pitch_ = 0.0f, distancia_ = 10.0f; // estado de la orbita
    CameraData data_{};
    float quieto_ = 0.0f;                  // [s] tiempo sin mover el mouse
    const float espera_retorno_ = 1.0f;    // [s] cuánto espera antes de volver
    const float velocidad_retorno_ = 2.0f; // más alto = vuelve más rápido
};