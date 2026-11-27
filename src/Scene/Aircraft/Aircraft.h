#pragma once

#include <map>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Mesh/Mesh.h"
#include "Texture/texture.h"
#include "RenderItem.h"

class Aircraft
{
public:
    void init(const std::string &ruta_modelo);          // arma piezas y locales, UNA VEZ
    void update(glm::vec3 pos, glm::vec3 angulos);      // recalcula la pose, cada cuadro
    void collect(std::vector<RenderItem> &items) const; // agrega un item por pieza al vector

private:
    std::vector<Mesh> piezas_;
    std::vector<glm::mat4> locales_; // una por pieza, fijas desde init()
    std::vector<glm::vec4> colores_;
    std::vector<const Texture *> texturas_;
    glm::mat4 pose_{1.0f};
    glm::vec3 reference_point_{0.0f}; // punto de referencia de la pose
    std::vector<Material> materiales_;
    void load_mesh(const MeshData &data);              // carga una malla y la agrega a piezas_
    std::map<std::string, Texture> texturas_cargadas_; // dueño de las texturas; texturas_ apunta acá
    const Texture *cargar_textura(ResourceManager &recursos, const std::string &nombre);
};