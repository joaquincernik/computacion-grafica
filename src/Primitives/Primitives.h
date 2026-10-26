#pragma once
#include <Mesh/Mesh.h>
#include "Utils/Alturas.h"
namespace primitives
{
    // 24 vertices, 36 indices, color por cara
    MeshData cube(float scale_x = 1.0f, float scale_y = 1.0f, float scale_z = 1.0f);
    MeshData cylinder(float radio, float largo, unsigned gajos, unsigned anillos = 1U);
    MeshData cone(float radio, float conicidad, unsigned gajos, unsigned anillos = 1U);
    MeshData terreno(int N, float step, const Alturas& alturas);
} // namespace primitives