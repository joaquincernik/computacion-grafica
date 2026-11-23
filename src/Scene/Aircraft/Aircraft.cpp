// Aircraft.cpp
#include "Aircraft.h"
#include "ObjLoader/ObjLoader.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Utils/Colors.h"
#include <cfloat>   // FLT_MAX
#include <cstdlib>  // exit
#include <iostream>

using glm::mat4;
using glm::vec3;
using std::move;
using std::vector;

static const char *kModeloPath = "src/asssets/avion.obj"; // relativo a la raiz del repo
static constexpr float kLargoAvion = 2.0f;                // largo final en unidades de escena

void Aircraft::init()
{
    piezas_.clear();
    locales_.clear();
    colores_.clear();
    reference_point_ = vec3(0.0f); // la matriz local ya deja el modelo centrado en el origen

    // Versiones "en tierra" de piezas que el archivo trae duplicadas (se dibujarian superpuestas).
    MeshData data = load_obj(kModeloPath, {"Landing_gear_down", "Doors_open", "Flaps_down", "slots_down"});
    if (data.vertices.empty())
    {
        std::cerr << "[Aircraft ERROR] No se pudo cargar el modelo: " << kModeloPath << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Bounding box: ANTES de load_mesh, despues los vertices viven en la GPU.
    vec3 mn(FLT_MAX), mx(-FLT_MAX);
    for (const Vertex &v : data.vertices)
    {
        mn = glm::min(mn, v.position);
        mx = glm::max(mx, v.position);
    }
    const vec3 centro = (mn + mx) * 0.5f;
    const float escala = kLargoAvion / (mx.z - mn.z);

    // Mlocal = R_y(180) * S * T(-centro): centrar, escalar y girar la nariz de +Z a -Z.
    mat4 local = glm::rotate(mat4(1.0f), glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
    local = glm::scale(local, vec3(escala));
    local = glm::translate(local, -centro);

    load_mesh(data);
    locales_.push_back(local);
    colores_.push_back(Colors::get("gray"));
}

void Aircraft::collect(vector<RenderItem> &items) const
{
    // piezas_[i], locales_[i] y colores_[i] describen la misma pieza.
    for (size_t i = 0; i < piezas_.size(); ++i)
    {
        RenderItem item;
        item.mesh = &piezas_[i];
        item.model = pose_ * locales_[i];
        item.color = colores_[i];
        items.push_back(item);
    }
}

void Aircraft::update(vec3 pos, vec3 angulos)
{
    // angulos = (phi, theta, psi) en radianes, ya en ejes GL (ver to_world()).
    // Mpose = T(pos) * R_y(psi) * R_x(theta) * R_z(phi) * T(-ref)
    mat4 pose = glm::translate(mat4(1.0f), pos);                  // T(pos)
    pose = glm::rotate(pose, angulos.z, vec3(0.0f, 1.0f, 0.0f)); // yaw   (psi)   sobre Y
    pose = glm::rotate(pose, angulos.y, vec3(1.0f, 0.0f, 0.0f)); // pitch (theta) sobre X
    pose = glm::rotate(pose, angulos.x, vec3(0.0f, 0.0f, 1.0f)); // roll  (phi)   sobre Z
    pose = glm::translate(pose, -reference_point_);              // T(-ref)
    pose_ = pose;
}

void Aircraft::load_mesh(const MeshData &data)
{
    Mesh mesh;
    mesh.load(data);
    piezas_.push_back(move(mesh));
}