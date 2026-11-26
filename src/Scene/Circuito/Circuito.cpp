#include "Circuito.h"
#include "Primitives/Primitives.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void Circuito::init()
{
    malla_aro_.load(primitives::torus(kRadioAro, kTubo, 48, 12));
    malla_bandera_.load(primitives::disco(kRadioAro - kTubo, 48));
    constexpr int kCantidad = 10;
    constexpr float kRadioCircuito = 3000.0f; // [m] radio de la vuelta
    const glm::vec3 centro(kRadioCircuito, 5000.0f, -1500.0f); // asi el aro 0 cae en (0, 5000, -1500)
    for (int i = 0; i < kCantidad; ++i)
    {
        const float a = 2.0f * glm::pi<float>() * float(i) / float(kCantidad);
        const glm::vec3 p(centro.x - kRadioCircuito * cos(a),
                          centro.y + 200.0f * sin(3.0f * a), // tres subidas y bajadas de +-200 m
                          centro.z - kRadioCircuito * sin(a));
        aros_.push_back({p, glm::degrees(a)});
    }
}

void Circuito::collect(std::vector<RenderItem> &items) const
{
    if (activo_ >= aros_.size())
        return; // circuito terminado

    RenderItem item;
    item.mesh = &malla_aro_;
    item.model = matriz(aros_[activo_]);
    item.material = mat_activo_;
    items.push_back(item);

    if (activo_ + 1 < aros_.size())
    {
        item.model = matriz(aros_[activo_ + 1]);
        item.material = mat_siguiente_;
        items.push_back(item);
    }
}

glm::mat4 Circuito::matriz(const Aro &aro) const
{
    // Implementación de la matriz de transformación para un aro
    glm::mat4 m = glm::translate(glm::mat4(1.0f), aro.centro);
    return glm::rotate(m, glm::radians(-aro.rumbo), glm::vec3(0.0f, 1.0f, 0.0f));
}

void Circuito::update(const glm::vec3 &pos, double ahora)
{
    if (hay_anterior_ && activo_ < aros_.size())
    {
        const Aro &aro = aros_[activo_];
        const float r = glm::radians(aro.rumbo);
        const glm::vec3 n(sin(r), 0.0f, -cos(r)); // sentido de cruce

        const float d0 = glm::dot(pos_anterior_ - aro.centro, n); // < 0: antes del plano
        const float d1 = glm::dot(pos - aro.centro, n);           // >= 0: despues del plano
        if (d0 < 0.0f && d1 >= 0.0f)
        {
            const float t = d0 / (d0 - d1); // fraccion del segmento donde corta el plano
            const glm::vec3 cruce = glm::mix(pos_anterior_, pos, t);
            if (glm::length(cruce - aro.centro) < kRadioAro - kTubo) // por adentro del tubo
            {
                ++activo_;
                if (activo_ == 1)
                    t_inicio_ = ahora; // paso el primer aro: arranca el reloj
                if (activo_ == aros_.size())
                    t_fin_ = ahora;
                std::cout << "Aro " << activo_ << "/" << aros_.size() << std::endl;
                if (activo_ == aros_.size())
                    std::cout << "Circuito completado!" << std::endl;
            }
        }
    }
    pos_anterior_ = pos;
    hay_anterior_ = true;
}

double Circuito::tiempo(double ahora) const
{
    if (activo_ == 0)
        return 0.0;
    return (terminado() ? t_fin_ : ahora) - t_inicio_;
}

const Mesh *Circuito::bandera(glm::mat4 &model) const
{
    if (aros_.empty())
        return nullptr;
    const size_t ultimo = aros_.size() - 1;
    // los mismos dos casos en que collect() dibuja el ultimo aro: como activo o como siguiente
    if (activo_ != ultimo && activo_ + 1 != ultimo)
        return nullptr;
    model = matriz(aros_[ultimo]); // misma matriz que el aro: el disco esta en XY como el toro
    return &malla_bandera_;
}