#include "CameraSystem/CameraData.h"
#include "CameraSystem/CameraCommand.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CameraSystem
{
public:
    CameraSystem(int width, int height);      // arma la proyeccion inicial
    void update(glm::vec3 pos_avion,          // donde esta el objetivo
                glm::vec3 angulos_avion,      // no se usa por ahora (ver Parte 3)
                const CameraCommand &cmd);    // se actualiza en cada cuadro
    void set_viewport(int width, int height); // al redimensionar la ventana
    const CameraData &data() const;           // los datos para construir la proyección
private:
    float yaw_ = 0.0f, pitch_ = 0.0f, distancia_ = 10.0f; // estado de la orbita
    CameraData data_{};
};