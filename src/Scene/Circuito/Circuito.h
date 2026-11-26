#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <Mesh/Mesh.h>
#include "Scene/Aircraft/RenderItem.h"

struct Aro
{
    glm::vec3 centro;
    float rumbo; // [grados] hacia donde hay que cruzarlo, 0 = norte (-Z), 90 = este (+X)
};

class Circuito
{
public:
    void init();
    void collect(std::vector<RenderItem> &items) const;
    void update(const glm::vec3 &pos, double ahora);
    double tiempo(double ahora) const; // 0 si no arranco, congelado si termino
    bool terminado() const { return activo_ == aros_.size(); }
    size_t pasados() const { return activo_; }
    size_t total() const { return aros_.size(); }
    const Mesh *bandera(glm::mat4 &model) const;
    const Aro *proximo() const { return terminado() ? nullptr : &aros_[activo_]; }

private:
    static constexpr float kRadioAro = 40.0f; // radio del aro
    static constexpr float kTubo = 2.f;

    std::vector<Aro> aros_;
    size_t activo_ = 0; // índice del aro activo

    Mesh malla_aro_;

    Material mat_activo_{glm::vec3(1.0f, 0.6f, 0.0f), glm::vec3(0.3f), 16.0f}; // naranja con brillo
    Material mat_siguiente_{glm::vec3(0.4f), glm::vec3(0.0f), 1.0f};           // gris mate

    glm::mat4 matriz(const Aro &aro) const; // T(centro) * R_y(-rumbo)
    glm::vec3 pos_anterior_{0.0f};
    bool hay_anterior_ = false;

    double t_inicio_ = 0.0;
    double t_fin_ = 0.0;
    Mesh malla_bandera_;
};
