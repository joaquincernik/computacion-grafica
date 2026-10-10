#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Colors 
{
    // Diccionario de colores
    static const std::unordered_map<std::string, glm::vec4> palette = {
        {"red",   glm::vec4(0.85f, 0.1f,  0.1f,  1.0f)},
        {"green", glm::vec4(0.3f,  0.8f,  0.35f, 1.0f)},
        {"blue",  glm::vec4(0.3f,  0.3f,  0.8f,  1.0f)},
        {"gray",  glm::vec4(0.3f,  0.3f,  0.35f, 1.0f)},
        {"white", glm::vec4(0.9f,  0.9f,  0.9f,  1.0f)},
        {"black", glm::vec4(0.1f,  0.1f,  0.1f,  1.0f)}
    };

    // Función para obtener el color de forma segura
    inline glm::vec4 get(const std::string& name) 
    {
        auto it = palette.find(name);
        if (it != palette.end()) {
            return it->second;
        }
        // Retorna blanco por defecto si te equivocas de nombre
        return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); 
    }
}