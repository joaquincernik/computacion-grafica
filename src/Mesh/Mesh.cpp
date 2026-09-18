#include <cstddef>
#include "Mesh.h"
#include <glad/gl.h>
#include <utility> // Para std::swap

Mesh::~Mesh()
{
    clear();
}
Mesh::Mesh(Mesh &&other) noexcept : vao_(other.vao_), vbo_(other.vbo_), ebo_(other.ebo_), count_(other.count_)
{
    other.vao_ = other.vbo_ = other.ebo_ = 0U; // se anula el origen
    other.count_ = 0;
}
void Mesh::load(const MeshData &data)
{
    // Si ya existía un VAO previo, limpiamos primero los recursos
    clear();

    count_ = static_cast<int>(data.indices.size());

    // 1. Crear objetos en la GPU
    glCreateVertexArrays(1, &vao_);
    glCreateBuffers(1, &vbo_);
    glCreateBuffers(1, &ebo_); // <-- Tu código de creación de EBO

    // 2. Subir datos de vértices a la GPU (VBO)
    glNamedBufferData(vbo_, data.vertices.size() * sizeof(Vertex), data.vertices.data(), GL_STATIC_DRAW);

    // 3. Subir datos de índices a la GPU (EBO)
    glNamedBufferData(ebo_, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);

    // 4. Conectar VBO y EBO al VAO
    glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(Vertex));
    glVertexArrayElementBuffer(vao_, ebo_); // <-- Tu conexión al VAO

    // 5. Configurar los atributos de vértice (Posición y Color)
    glVertexArrayAttribFormat(vao_, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, px));
    glVertexArrayAttribBinding(vao_, 0, 0);
    glEnableVertexArrayAttrib(vao_, 0);

    glVertexArrayAttribFormat(vao_, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, r));
    glVertexArrayAttribBinding(vao_, 1, 0);
    glEnableVertexArrayAttrib(vao_, 1);
}
void Mesh::clear(void)
{
    glDeleteVertexArrays(1, &vao_); // glDelete *(0) es legal : no hace nada
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
    vao_ = vbo_ = ebo_ = 0U;
    count_ = 0;
}