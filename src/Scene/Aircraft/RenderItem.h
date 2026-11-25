#pragma once

#include <glm/glm.hpp>
#include <Mesh/Mesh.h>

class Texture;

struct RenderItem
{
    const Mesh *mesh = nullptr;
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec4 color = glm::vec4(1.0f);
    const Texture *texture = nullptr;  // nullptr = color plano
};