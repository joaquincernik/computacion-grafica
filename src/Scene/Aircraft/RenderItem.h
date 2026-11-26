#pragma once

#include <glm/glm.hpp>
#include <Mesh/Mesh.h>
#include <Material/Material.h>

class Texture;

struct RenderItem
{
    const Mesh *mesh = nullptr;
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec4 color = glm::vec4(1.0f); // color plano si texture == nullptr
    Material material;
    const Texture *texture = nullptr;  // nullptr = color plano

};
// Al dibujar cada pieza. El shader de la aeronave declara: uniform Material uMaterial;
/*oid enviar_material(GLuint prog, const Material &m)
{
    glProgramUniform3fv(prog, glGetUniformLocation(prog, "uMaterial.kd"), 1, glm::value_ptr(m.kd));
    glProgramUniform3fv(prog, glGetUniformLocation(prog, "uMaterial.ks"), 1, glm::value_ptr(m.ks));
    glProgramUniform1f(prog, glGetUniformLocation(prog, "uMaterial.ns"), m.ns);
}*/