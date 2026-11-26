// Material.h - los mismos tres campos que el struct Material de iluminacion.glsl
#pragma once
#include <glm/glm.hpp>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

using glm::vec3;
using glm::value_ptr;

struct Material
{
    glm::vec3 kd = glm::vec3(1.0f); // el color que antes llevaba el RenderItem
    glm::vec3 ks = glm::vec3(0.0f);
    float ns = 1.0f;
};
