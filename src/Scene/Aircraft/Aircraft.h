#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Mesh/Mesh.h>
#include "RenderItem.h"

class Aircraft
{
public:
    void init();                                        // arma piezas y locales, UNA VEZ
    void update(glm::vec3 pos, glm::vec3 angulos);      // recalcula la pose, cada cuadro
    void collect(std::vector<RenderItem> &items) const; // agrega un item por pieza al vector

private:
    std::vector<Mesh> piezas_;
    std::vector<glm::mat4> locales_;  // una por pieza, fijas desde init()
    std::vector<glm::vec4> colores_;
    glm::mat4 pose_{1.0f};
    glm::vec3 reference_point_{0.0f}; // punto de referencia de la pose
};