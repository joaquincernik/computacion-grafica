// Aircraft.cpp
#include "Aircraft.h"
#include "Primitives/Primitives.h"
#include <glm/gtc/matrix_transform.hpp>

using glm::mat4;
using glm::vec3;
using std::move;
using std::vector;

void Aircraft::init()
{
    piezas_.clear();
    locales_.clear();
    colores_.clear();

    // --- 1. FUSELAJE (Cilindro) ---
    // cylinder() genera con base en y = 0 y tapa en y = largo (Primitives.cpp).
    // GLM post-multiplica: translate y luego rotate dan M = T(0,0,1) * R(-90,X).
    // R(-90,X) lo acuesta sobre Z (z in [-2, 0]) y T(0,0,1) lo centra en z in [-1, 1].
    const float radio_fuselaje = 0.2f;
    const float largo_fuselaje = 2.0f;

    MeshData data_cilindro = primitives::cylinder(radio_fuselaje, largo_fuselaje, 32);
    Mesh mesh_cilindro;
    mesh_cilindro.load(data_cilindro);
    piezas_.push_back(move(mesh_cilindro));

    mat4 local_fuselaje = glm::translate(mat4(1.0f), vec3(0.0f, 0.0f, 1.0f));
    local_fuselaje = glm::rotate(local_fuselaje, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
    locales_.push_back(local_fuselaje);
    colores_.push_back(glm::vec4(0.3f, 0.3f, 0.35f, 1.0f)); // Gris

    // --- 2. NARIZ (Cono) ---
    // M = T(0,0,-0.95) * R(-90,X): base apoyada en la nariz del fuso (z=-0.95)
    // y apice hacia -Z (adelante).
    MeshData data_cono = primitives::cone(radio_fuselaje, glm::radians(45.0f), 32);
    Mesh mesh_cono;
    mesh_cono.load(data_cono);
    piezas_.push_back(move(mesh_cono));

    mat4 local_nariz = glm::translate(mat4(1.0f), vec3(0.0f, 0.0f, -0.95f));
    local_nariz = glm::rotate(local_nariz, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
    locales_.push_back(local_nariz);
    colores_.push_back(glm::vec4(0.8f, 0.3f, 0.35f, 1.0f)); // Rojo

    // --- 3. CUBO reutilizado para alas y cola ---
    MeshData cube_data = primitives::cube(1.0f); // Cubo unitario
    Mesh cube_mesh;
    cube_mesh.load(cube_data);
    piezas_.push_back(move(cube_mesh));

    // Ala izquierda / derecha (simetricas, sin escala negativa).
    // Ala baja: apoyada en el vientre del fuso (radio 0.2). El centro va en y=-0.12
    // para que la raiz entre al fuso y emerja por el borde inferior (no por el eje).
    mat4 local_ala_izq = glm::translate(mat4(1.0f), vec3(-0.65f, -0.12f, 0.1f));
    local_ala_izq = glm::scale(local_ala_izq, vec3(1.25f, 0.05f, 0.7f));
    locales_.push_back(local_ala_izq);
    colores_.push_back(glm::vec4(0.3f, 0.8f, 0.35f, 1.0f)); // Verde

    mat4 local_ala_der = glm::translate(mat4(1.0f), vec3(0.65f, -0.12f, 0.1f));
    local_ala_der = glm::scale(local_ala_der, vec3(1.25f, 0.05f, 0.7f));
    locales_.push_back(local_ala_der);
    colores_.push_back(glm::vec4(0.3f, 0.8f, 0.35f, 1.0f)); // Verde

    // Empenaje horizontal: aletas simetricas montadas sobre el lomo (y=+0.14)
    mat4 local_aleta_izq = glm::translate(mat4(1.0f), vec3(-0.35f, 0.14f, 0.75f));
    local_aleta_izq = glm::scale(local_aleta_izq, vec3(0.5f, 0.05f, 0.4f));
    locales_.push_back(local_aleta_izq);
    colores_.push_back(glm::vec4(0.3f, 0.3f, 0.8f, 1.0f)); // Azul

    mat4 local_aleta_der = glm::translate(mat4(1.0f), vec3(0.35f, 0.14f, 0.75f));
    local_aleta_der = glm::scale(local_aleta_der, vec3(0.5f, 0.05f, 0.4f));
    locales_.push_back(local_aleta_der);
    colores_.push_back(glm::vec4(0.3f, 0.3f, 0.8f, 1.0f)); // Azul

    // Timon vertical: base en y=0.18, dentro de la tapa del fuso (0.2), para no dejar hueco
    mat4 local_timon = glm::translate(mat4(1.0f), vec3(0.0f, 0.53f, 0.7f));
    local_timon = glm::scale(local_timon, vec3(0.05f, 0.7f, 0.35f));
    locales_.push_back(local_timon);
    colores_.push_back(glm::vec4(0.3f, 0.3f, 0.8f, 1.0f)); // Azul

    // Escala global de encuadre: sin camara, el clip-space es el NDC ([-1,1]^3) y todo
    // lo que quede afuera se recorta. El fuso ya ocupa z=±1, asi que la nariz y las
    // puntas salian del volumen. Escalamos todo el modelo uniformemente (~0.75) para
    // que entre completo, sin deformar (max|clip| pasa de 1.17 a ~0.87).
    const float kEscala = 0.75f;
    for (mat4 &local : locales_)
        local = glm::scale(mat4(1.0f), vec3(kEscala)) * local;
}

void Aircraft::collect(vector<RenderItem> &items) const
{
    // piezas_ tiene 3 mallas (fuso, nariz, cubo) y locales_ tiene 7 items.
    // Varios items comparten la malla del cubo: esta es la razon por la que
    // la cantidad de mallas y la cantidad de matrices locales no coincide.
    // Mapa item -> malla:
    //   0 fuso, 1 nariz, 2 ala izq, 3 ala der, 4 aleta izq, 5 aleta der, 6 timon
    const size_t mapa_mallas[] = {0, 1, 2, 2, 2, 2, 2};
    const size_t n_items = locales_.size();

    for (size_t i = 0; i < n_items; ++i)
    {
        if (i >= 7)
            break; // el mapa solo cubre las 7 piezas actuales

        RenderItem item;
        item.model = pose_ * locales_[i];
        item.color = colores_[i];
        item.mesh = &piezas_[mapa_mallas[i]];
        items.push_back(item);
    }
}

void Aircraft::update(vec3 pos, vec3 angulos)
{
    // Mpose = T(pos) * R(roll)*R(pitch)*R(yaw) * T(-ref)
    mat4 pose = glm::translate(mat4(1.0f), pos);                         // T(pos)
    pose = glm::rotate(pose, glm::radians(angulos.y), vec3(0.0f, 1.0f, 0.0f)); // yaw (Y)
    pose = glm::rotate(pose, glm::radians(angulos.x), vec3(1.0f, 0.0f, 0.0f)); // pitch (X)
    pose = glm::rotate(pose, glm::radians(angulos.z), vec3(0.0f, 0.0f, 1.0f)); // roll (Z)
    pose = glm::translate(pose, -reference_point_);                     // T(-ref)
    pose_ = pose;
}