#pragma once

#include <string>
// Es la misma estrategia planteada para el módulo Mesh sobre otro recurso: un programa de shader que
// se identifica por un nombre (un único entero en vez de tres como en mesh (ebo,vao,vbo)). Lo que cambia es el tipo de recurso que
// se administra.
class Shader
{
public:
    Shader() = default;
    ~Shader();                       // libera el programa
    Shader(const Shader &) = delete; // las MISMAS tres decisiones
    Shader &operator=(const Shader &) = delete;
    Shader(Shader &&other) noexcept;
    Shader &operator=(Shader &&other) noexcept;
    bool compile_from_source(const std::string &vs, const std::string &fs);
    void use(void) const;
    void clear(void);
    unsigned int id(void) const { return id_; }

private:
    unsigned int id_{0U};
};