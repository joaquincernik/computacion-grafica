#include "CameraSystem.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

CameraSystem::CameraSystem(int width, int height)
{
    set_viewport(width, height);
}

const CameraData &CameraSystem::data() const
{
    return data_;
}

void CameraSystem::update(glm::vec3 pos_avion, glm::vec3 angulos_avion, const CameraCommand &cmd)
{
    // Actualizar el estado de la cámara según los comandos recibidos
    yaw_ += cmd.yaw_delta;
    pitch_ += cmd.pitch_delta;
    distancia_ += cmd.dist_delta;

    // Limitar el pitch para evitar que la cámara se voltee
    if (pitch_ > radians(89.0f))
        pitch_ = radians(89.0f);
    if (pitch_ < radians(-89.0f))
        pitch_ = radians(-89.0f);

    // Calcular la posición de la cámara en coordenadas esféricas
    float x = distancia_ * cos(pitch_) * cos(yaw_);
    float y = distancia_ * sin(pitch_);
    float z = distancia_ * cos(pitch_) * sin(yaw_);

    // Establecer la posición de la cámara relativa al avión
    vec3 eye = pos_avion + vec3(x, y, z);

    // Calcular la matriz de vista usando lookAt
    data_.view = lookAt(eye, pos_avion, vec3(0.0f, 1.0f, 0.0f));
};

void CameraSystem::set_viewport(int width, int height)
{
    if (height > 0)
    { // Protegemos el código para evitar dividir por cero si minimizas la ventana
        data_.projection = perspective(radians(45.0f), (float)width / height, 0.1f, 100.0f);
    }
}