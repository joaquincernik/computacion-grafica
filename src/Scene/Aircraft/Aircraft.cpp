// Aircraft.cpp
#include "Aircraft.h"
#include "Primitives/Primitives.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Utils/Colors.h"

using glm::mat4;
using glm::vec3;
using std::move;
using std::vector;

void Aircraft::init(const Texture *textura)
{
    piezas_.clear();
    locales_.clear();
    colores_.clear();
    texturas_.clear();
    materiales_.clear();

    reference_point_ = glm::vec3(0.0f, 0.0f, 1.1f);
    const float radio_fuselaje = 0.2f;
    const float largo_fuselaje = 2.0f;

    const vec3 eje_x = vec3(1.0f, 0.0f, 0.0f);
    const vec3 dimensiones_ala = vec3(1.25f, 0.05f, 0.7f);
    const vec3 dimensiones_aleta = vec3(0.5f, 0.05f, 0.4f);

    const float posicion_ala_izq = -0.65f;
    const float posicion_aleta_izq = 0.35f;
    const float final_z = 2.0f; // z final del fuselaje, para ubicar las aletas y el timón


    // carga de primitivas en mesh
    load_mesh(primitives::cylinder(radio_fuselaje, largo_fuselaje, 32));
    load_mesh(primitives::cube(1.0f));
    load_mesh(primitives::cone(radio_fuselaje, glm::radians(45.0f), 32));

    // --- 1. NARIZ (Cono) ---
    mat4 local_nariz = glm::translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.2f));
    local_nariz = glm::rotate(local_nariz, glm::radians(-90.0f), eje_x);
    locales_.push_back(local_nariz);
    colores_.push_back(Colors::get("red"));
    materiales_.push_back(Material{ vec3(Colors::get("red")), vec3(0.55f), 64.0f });
    texturas_.push_back(textura);

    // 2 - FUSELAJE
    mat4 local_fuselaje = glm::translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.2f));
    local_fuselaje = glm::rotate(local_fuselaje, glm::radians(90.0f), eje_x);
    locales_.push_back(local_fuselaje);
    colores_.push_back(Colors::get("gray"));
    materiales_.push_back(Material{ vec3(Colors::get("gray")), vec3(0.45f), 48.0f });
    texturas_.push_back(textura);

    // --- 3. CUBO reutilizado para alas y cola ---
    Material mat_ala{ vec3(Colors::get("green")), vec3(0.25f), 24.0f };

    mat4 local_ala_izq = glm::translate(mat4(1.0f), vec3(posicion_ala_izq, -0.12f, 1.0f));
    local_ala_izq = glm::scale(local_ala_izq, dimensiones_ala);
    locales_.push_back(local_ala_izq);
    colores_.push_back(Colors::get("green"));
    materiales_.push_back(mat_ala);
    texturas_.push_back(textura);

    mat4 local_ala_der = glm::translate(mat4(1.0f), vec3(-posicion_ala_izq, -0.12f, 1.0f));
    local_ala_der = glm::scale(local_ala_der, dimensiones_ala);
    locales_.push_back(local_ala_der);
    colores_.push_back(Colors::get("green"));
    materiales_.push_back(mat_ala);
    texturas_.push_back(textura);

    // aletas
    Material mat_cola{ vec3(Colors::get("blue")), vec3(0.25f), 24.0f };

    mat4 local_aleta_izq = glm::translate(mat4(1.0f), vec3(-posicion_aleta_izq, 0.0f, final_z));
    local_aleta_izq = glm::scale(local_aleta_izq, dimensiones_aleta);
    locales_.push_back(local_aleta_izq);
    colores_.push_back(Colors::get("blue"));
    materiales_.push_back(mat_cola);
    texturas_.push_back(textura);

    mat4 local_aleta_der = glm::translate(mat4(1.0f), vec3(posicion_aleta_izq, 0.0f, final_z));
    local_aleta_der = glm::scale(local_aleta_der, dimensiones_aleta);
    locales_.push_back(local_aleta_der);
    colores_.push_back(Colors::get("blue"));
    materiales_.push_back(mat_cola);
    texturas_.push_back(textura);

    // Timon vertical
    mat4 local_timon = glm::translate(mat4(1.0f), vec3(0.0f, 0.50f, final_z));
    local_timon = glm::scale(local_timon, vec3(0.05f, 0.7f, 0.35f));
    locales_.push_back(local_timon);
    colores_.push_back(Colors::get("blue"));
    materiales_.push_back(mat_cola);
    texturas_.push_back(textura);
}

void Aircraft::collect(vector<RenderItem> &items) const
{

    //   0 fuso, 1 nariz, 2 ala izq, 3 ala der, 4 aleta izq, 5 aleta der, 6 timon
    const size_t mapa_mallas[] = {2, 0, 1, 1, 1, 1, 1};
    const size_t n_items = locales_.size();

    for (size_t i = 0; i < n_items; ++i)
    {
        if (i >= 7)
            break; // el mapa solo cubre las 7 piezas actuales

        RenderItem item;
        item.model = pose_ * locales_[i];
        item.color = colores_[i];
        item.material = materiales_[i];
        item.texture = texturas_[i];
        item.mesh = &piezas_[mapa_mallas[i]];
        items.push_back(item);
    }
}

void Aircraft::update(vec3 pos, vec3 angulos)
{
    // angulos = (phi, theta, psi) en radianes, ya en ejes OpenGL (ver to_world)
    // Mpose = T(pos) * R(yaw)*R(pitch)*R(roll) * T(-ref)
    mat4 pose = glm::translate(mat4(1.0f), pos);                  // T(pos)
    pose = glm::rotate(pose, angulos.z, vec3(0.0f, 1.0f, 0.0f)); // yaw   psi   (Y)
    pose = glm::rotate(pose, angulos.y, vec3(1.0f, 0.0f, 0.0f)); // pitch theta (X)
    pose = glm::rotate(pose, angulos.x, vec3(0.0f, 0.0f, 1.0f)); // roll  phi   (Z)
    pose = glm::translate(pose, -reference_point_);                            // T(-ref)
    pose_ = pose;
}

void Aircraft::load_mesh(const MeshData &data)
{
    Mesh mesh;
    mesh.load(data);
    piezas_.push_back(move(mesh));
}