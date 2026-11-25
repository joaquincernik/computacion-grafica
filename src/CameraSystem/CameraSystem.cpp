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

    float theta = angulos_avion.y; // cabeceo
    float psi = angulos_avion.z;   // rumbo

    vec3 adelante(-sin(psi) * cos(theta),
                  sin(theta),
                  -cos(psi) * cos(theta));

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
};

void CameraSystem::set_viewport(int width, int height)
{
    if (height > 0)
        data_.projection = perspective(radians(45.0f), (float)width / height, 0.1f, 10000.0f); // Protegemos el código para evitar dividir por cero si minimizas la ventana
}