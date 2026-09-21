#pragma once

#include <string>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
    unsigned int id(void) const { return id_; };
    void set_uniform(const std::string &nombre, const glm::mat4 &m) const;
    void set_uniform(const std::string &nombre, const glm::vec3 &v) const;

private:
    GLuint crear_vertex_shader_GPU(const char *vs_code);
    GLuint crear_fragment_shader_GPU(const char *fs_code);
    GLuint crear_program_GPU(const GLuint vertex_shader, const GLuint fragment_shader); // program es el ejecutable final en lagpu
    void borrarObjetosIntermedios(const GLuint vertex_shader, const GLuint fragment_shader, const GLuint program);

    unsigned int id_{0U};
};