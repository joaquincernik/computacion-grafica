#pragma once
#include "CameraSystem/CameraData.h"
#include "CameraSystem/CameraCommand.h"
#include <glm/glm.hpp>
#include <random>
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

    enum class Modo
    {
        Orbita,
        Cine
    };
    enum class Toma
    {
        Pasada,
        Tele,
        Lateral
    };

    void orbita(glm::vec3 pos_avion, float psi, const CameraCommand &cmd, float dt); // la camara de siempre
    void cine(glm::vec3 pos_avion, glm::vec3 adelante, float dt);                    // tomas con cortes
    void nueva_toma(glm::vec3 pos_avion, glm::vec3 adelante, glm::vec3 lado);        // sortea la siguiente

    Modo modo_ = Modo::Orbita;
    float aspecto_ = 1.0f; // se guarda para rearmar la proyeccion cuando cambia el fov
    float fov_ = glm::radians(45.0f);

    // estado del modo cine
    Toma toma_ = Toma::Pasada;
    float t_toma_ = 0.0f;      // [s] que le queda a la toma actual
    float signo_ = 1.0f;       // de que lado del avion esta la camara (+1 der, -1 izq)
    glm::vec3 ojo_cine_{0.0f}; // ojo fijo (pasada y tele)
    glm::vec3 offset_{0.0f};   // ojo relativo al avion (lateral), suavizado
    std::mt19937 rng_{std::random_device{}()};
};