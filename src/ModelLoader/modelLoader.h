#pragma once

#include <string>
#include <vector>
#include "Mesh/Mesh.h"
#include "Material/Material.h"

struct ModelPart
{
    MeshData data;
    Material material;
    std::string textura; // "" si la parte no tiene textura
};
// Carga cualquier formato que lea assimp (.ac, .obj, ...). Una parte por malla de assimp.
std::vector<ModelPart> load_model(const std::string &path, const std::vector<std::string> &ignorar = {});