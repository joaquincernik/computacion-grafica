#pragma once

#include <vector>
#include "Vertex.h"

struct MeshData
{
    std::vector<Vertex> vertices;      // conjunto mí nimo requerido
    std::vector<unsigned int> indices; // orden de lectura para formar triangulos
};

class Mesh
{
public:
    Mesh() = default;
    ~Mesh();                     // libera lo que posee
    Mesh(const Mesh &) = delete; // copiar rompe el due ño ú nico
    Mesh &operator=(const Mesh &) = delete;
    Mesh(Mesh &&other) noexcept; // mover : se traspasa la propiedad
    Mesh &operator=(Mesh &&other) noexcept;

    // load() crea el VAO, el VBO y el EBO, sube las dos listas y configura los dos atributos (posición
    // y color) sobre un único buffer entrelazado.
    void load(const MeshData &data);
    
    void clear(void);
    unsigned int vao(void) const { return vao_; }
    int count(void) const { return count_; }
    // el tema de las copias se h ace  todo esa manipulacion de funciones para que si jago una copia de un mesh
    //  al eliminar una no se eliminen las dos
private:
    unsigned int vao_{0U}, vbo_{0U}, ebo_{0U};
    int count_{0};
};