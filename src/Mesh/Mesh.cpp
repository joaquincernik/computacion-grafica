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
    count_ = static_cast<int>(data.indices.size()); // cantidad total de indices del modelo

    // 1. Crear objetos en la GPU
    crear_objetos_GPU();

    // 2. Subir datos de vértices a la GPU
    upload_vertex_data_GPU(data);

    // 3. Conectar VBO y EBO al VAO y configurar atrivutos del vertex
    configurar_vao();
}
void Mesh::clear(void)
{
    if (vao_ != 0U || vbo_ != 0U || ebo_ != 0U)
    {
        glDeleteVertexArrays(1, &vao_);
        glDeleteBuffers(1, &vbo_);
        glDeleteBuffers(1, &ebo_);
    }
    vao_ = vbo_ = ebo_ = 0U;
    count_ = 0;
}

// funciones auxiliares

void Mesh::crear_objetos_GPU()
{
    glCreateVertexArrays(1, &vao_);
    glCreateBuffers(1, &vbo_);
    glCreateBuffers(1, &ebo_);
}

void Mesh::upload_vertex_data_GPU(const MeshData &data)
{
    // subida del vbo
    glNamedBufferData(vbo_, data.vertices.size() * sizeof(Vertex), data.vertices.data(), GL_STATIC_DRAW);
    // subida del ebo
    glNamedBufferData(ebo_, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);

    /*solo subimos estos dos porque son los unicos que contienen datos crusod en la memoria de la gpu
    el VAO no almacena nada, es solo un mapa de conf9ig
    */
}

void Mesh::configurar_vao()
{
    /*asocia el buffer vbo_ al punto de amarre indice 0 dentro del vao comenznaod en byte 0
    y caminando a pasos de size of vertex*/
    glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(Vertex)); // stride
    glVertexArrayElementBuffer(vao_, ebo_);

    // Configurar los atributos de vértice
    glVertexArrayAttribFormat(vao_, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    glVertexArrayAttribBinding(vao_, 0, 0);
    glEnableVertexArrayAttrib(vao_, 0);

    glVertexArrayAttribFormat(vao_, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
    glVertexArrayAttribBinding(vao_, 1, 0);
    glEnableVertexArrayAttrib(vao_, 1);

    // se agrega esto nomas
    glVertexArrayAttribFormat(vao_, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_coords));
    glVertexArrayAttribBinding(vao_, 2, 0);
    glEnableVertexArrayAttrib(vao_, 2);
}