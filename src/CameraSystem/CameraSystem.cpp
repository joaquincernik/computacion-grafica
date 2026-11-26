#include "CameraSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

CameraSystem::CameraSystem(int width, int height)
{
    set_viewport(width, height);
}

const CameraData &CameraSystem::data() const
{
    return data_;
}

void CameraSystem::update(glm::vec3 pos_avion, glm::vec3 angulos_avion, const CameraCommand &cmd, float dt)
{
    if (cmd.cambiar_modo)
    {
        modo_ = (modo_ == Modo::Orbita) ? Modo::Cine : Modo::Orbita;
        t_toma_ = 0.0f; // al entrar al cine fuerza una toma nueva
    }

    float theta = angulos_avion.y; // cabeceo
    float psi = angulos_avion.z;   // rumbo

    vec3 adelante(-sin(psi) * cos(theta),
                  sin(theta),
                  -cos(psi) * cos(theta));

    if (modo_ == Modo::Orbita)
        orbita(pos_avion, psi, cmd, dt);
    else
        cine(pos_avion, adelante, dt);

    data_.projection = perspective(fov_, aspecto_, 0.1f, 10000.0f); // el fov puede cambiar en cine
}

void CameraSystem::orbita(glm::vec3 pos_avion, float psi, const CameraCommand &cmd, float dt)
{
    fov_ = radians(45.0f); // al volver del cine, deshace el zoom de la toma tele

    // Actualizar el estado de la cámara según los comandos recibidos
    bool moviendo = (cmd.yaw_delta != 0.0f || cmd.pitch_delta != 0.0f);

    if (moviendo)
    {
        yaw_ += cmd.yaw_delta;
        pitch_ += cmd.pitch_delta;
        quieto_ = 0.0f;
    }
    else
    {
        quieto_ += dt;
        if (quieto_ > espera_retorno_)
        {
            float k = glm::min(1.0f, velocidad_retorno_ * dt);
            yaw_ -= yaw_ * k;
            pitch_ -= pitch_ * k;
        }
    }
    distancia_ += cmd.dist_delta;

    float yaw_base = radians(90.0f) - psi; // detrás del avión
    float pitch_base = radians(10.0f);     // un poco desde arriba

    float yaw_total = yaw_base + yaw_;
    float pitch_total = clamp(pitch_base + pitch_, radians(-89.0f), radians(89.0f));

    float x = distancia_ * cos(pitch_total) * cos(yaw_total);
    float y = distancia_ * sin(pitch_total);
    float z = distancia_ * cos(pitch_total) * sin(yaw_total);

    // Establecer la posición de la cámara relativa al avión
    vec3 eye = pos_avion + vec3(x, y, z);
    vec3 eje_y = vec3(0.0f, 1.0f, 0.0f); // Vector hacia arriba

    // Calcular la matriz de vista usando lookAt
    data_.view = lookAt(eye, pos_avion, eje_y);
    data_.eye = eye;
}

void CameraSystem::set_viewport(int width, int height)
{
    if (height > 0) // evita dividir por cero si minimizas la ventana
    {
        aspecto_ = (float)width / height;
        data_.projection = perspective(fov_, aspecto_, 0.1f, 10000.0f);
    }
}
void CameraSystem::cine(vec3 pos_avion, vec3 adelante, float dt)
{
    const vec3 arriba(0.0f, 1.0f, 0.0f);
    const vec3 lado = normalize(cross(adelante, arriba)); // derecha del avion, horizontal

    t_toma_ -= dt;
    const bool se_fue = toma_ == Toma::Pasada && dot(pos_avion - ojo_cine_, adelante) > 600.0f; // ya paso y se alejo
    if (t_toma_ <= 0.0f || se_fue)
        nueva_toma(pos_avion, adelante, lado);

    vec3 ojo = ojo_cine_;
    fov_ = radians(45.0f);
    if (toma_ == Toma::Tele)
    {
        // zoom: el campo de vision encuadra ~50 m alrededor del avion a cualquier distancia
        const float dist = length(pos_avion - ojo_cine_);
        fov_ = clamp(2.0f * atan(25.0f / dist), radians(2.0f), radians(45.0f));
    }
    else if (toma_ == Toma::Lateral)
    {
        // el offset (no la posicion) se suaviza: si suavizara la posicion, a 150 m/s quedaria 50 m atras
        const vec3 objetivo = lado * (30.0f * signo_) + arriba * 5.0f - adelante * 10.0f;
        offset_ = mix(offset_, objetivo, 1.0f - exp(-2.0f * dt)); // suavizado independiente de los fps
        ojo = pos_avion + offset_;
    }

    data_.view = lookAt(ojo, pos_avion, arriba);
    data_.eye = ojo;
}

void CameraSystem::nueva_toma(vec3 pos_avion, vec3 adelante, vec3 lado)
{
    const vec3 arriba(0.0f, 1.0f, 0.0f);
    // +1 o +2 sobre la actual: nunca repite la misma toma dos veces seguidas
    toma_ = Toma((int(toma_) + 1 + std::uniform_int_distribution<int>(0, 1)(rng_)) % 3);
    t_toma_ = std::uniform_real_distribution<float>(4.0f, 7.0f)(rng_);
    signo_ = std::uniform_int_distribution<int>(0, 1)(rng_) ? 1.0f : -1.0f;

    switch (toma_)
    {
    case Toma::Pasada: // clavada adelante sobre la trayectoria: el avion le pasa al lado
        ojo_cine_ = pos_avion + adelante * 400.0f + lado * (40.0f * signo_) +
                    arriba * std::uniform_real_distribution<float>(-15.0f, 25.0f)(rng_);
        break;
    case Toma::Tele: // lejos al costado y mas abajo, como filmado desde tierra
        ojo_cine_ = pos_avion + adelante * 300.0f + lado * (1500.0f * signo_) - arriba * 100.0f;
        break;
    case Toma::Lateral: // arranca ya en posicion, despues lo sigue suavizado
        offset_ = lado * (30.0f * signo_) + arriba * 5.0f - adelante * 10.0f;
        break;
    }
}