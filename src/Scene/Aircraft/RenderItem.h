#pragma once

#include <glm/glm.hpp>
#include <Mesh/Mesh.h>

struct RenderItem
{
    const Mesh *mesh = nullptr;        // malla a dibujar (referencia, no es propia)
    glm::mat4 model = glm::mat4(1.0f); // transformacion modelo -> mundo, YA compuesta
    glm::vec4 color = glm::vec4(1.0f);
};