// Aircraft.cpp
#include "Aircraft.h"
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include "ModelLoader/modelLoader.h"

using glm::mat4;
using glm::vec3;
using std::move;
using std::vector;

void Aircraft::init(const std::string &ruta_modelo)
{
    piezas_.clear();
    locales_.clear();
    texturas_.clear();
    materiales_.clear();
    texturas_cargadas_.clear();

    reference_point_ = vec3(0.0f);
    const float escala = 0.13f; // el F-16 mide ~17.5 m; asi queda de ~2.3 unidades

    // el modelo tiene el morro en -X: lo giramos para que mire a -Z
    mat4 local = glm::rotate(mat4(1.0f), glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
    local = glm::scale(local, vec3(escala));

    // las texturas del .ac son relativas a la carpeta del modelo
    ResourceManager recursos(std::filesystem::path(ruta_modelo).parent_path().string());
    // piezas que en FlightGear se animan o se esconden: aca no las queremos
    const vector<std::string> ocultar = {
        "ExternalFlame",
        "InternalFlame", // fuego del postquemador
        "FrontTire",
        "LeftMainTire",
        "RightMainTire", // ruedas
        "FrontAftStrut",
        "FrontLowerStrut",
        "FrontUpperStrut", // tren delantero
        "LeftLowerMainStrut",
        "LeftInnerStrut",
        "LeftOuterLowerStrut",
        "LeftOuterUpperStrut",
        "LeftUpperMainStrut",
        "RightLowerMainStrut",
        "RightInnerStrut",
        "RightOuterLowerStrut",
        "RightOuterUpperStrut",
        "RightUpperMainStrut",
    };
    for (const ModelPart &parte : load_model(ruta_modelo, ocultar))
    {
        load_mesh(parte.data);
        locales_.push_back(local);
        materiales_.push_back(parte.material);
        texturas_.push_back(cargar_textura(recursos, parte.textura));
    }
}

void Aircraft::collect(vector<RenderItem> &items) const
{

    //   0 fuso, 1 nariz, 2 ala izq, 3 ala der, 4 aleta izq, 5 aleta der, 6 timon
    const size_t mapa_mallas[] = {2, 0, 1, 1, 1, 1, 1};
    const size_t n_items = locales_.size();

    for (size_t i = 0; i < n_items; ++i)
    {
        RenderItem item;
        item.model = pose_ * locales_[i];
        item.material = materiales_[i];
        item.texture = texturas_[i];
        item.mesh = &piezas_[i];
        items.push_back(item);
    }
}

void Aircraft::update(vec3 pos, vec3 angulos)
{
    // angulos = (phi, theta, psi) en radianes, ya en ejes OpenGL (ver to_world)
    // Mpose = T(pos) * R(yaw)*R(pitch)*R(roll) * T(-ref)
    mat4 pose = glm::translate(mat4(1.0f), pos);                 // T(pos)
    pose = glm::rotate(pose, angulos.z, vec3(0.0f, 1.0f, 0.0f)); // yaw   psi   (Y)
    pose = glm::rotate(pose, angulos.y, vec3(1.0f, 0.0f, 0.0f)); // pitch theta (X)
    pose = glm::rotate(pose, angulos.x, vec3(0.0f, 0.0f, 1.0f)); // roll  phi   (Z)
    pose = glm::translate(pose, vec3(0.0f));                     // T(-ref)
    pose_ = pose;
}

void Aircraft::load_mesh(const MeshData &data)
{
    Mesh mesh;
    mesh.load(data);
    piezas_.push_back(move(mesh));
}
const Texture *Aircraft::cargar_textura(ResourceManager &recursos, const std::string &nombre)
{
    if (nombre.empty())
        return nullptr; // pieza sin textura: se dibuja con su color
    auto it = texturas_cargadas_.find(nombre);
    if (it == texturas_cargadas_.end())
    {
        const ImageData &img = recursos.load_image(nombre, nombre, true);
        if (img.pixels.empty())
            return nullptr; // el ResourceManager ya informo el error
        it = texturas_cargadas_.emplace(nombre, Texture{}).first;
        it->second.load_2d(img);
    }
    return &it->second;
}