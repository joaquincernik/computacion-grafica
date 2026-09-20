# Practico 3

## Tupla nueva en vertice

1. ¿Cuántos lugares del código hubo que tocar para agregar un atributo?

- `Vertex.h`: se agrega el atributo a la tupla

```c++
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};
```

- `Primitives.cpp`: adaptamos la primitiva con el nuevo atributo

```c++
data.vertices = {
 // CARA FRONTAL (+Z) -> Normal apuntando hacia +Z: {0.0f, 0.0f, 1.0f}
            {{-hx, -hy, hz}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // Inf-Izq (0)
            {{hx, -hy, hz}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},  // Inf-Der (1)
            {{hx, hy, hz}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // Sup-Der (2)
            {{-hx, hy, hz}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}  // Sup-Izq (3)
            };
            // (...)
```

- `Mesh.cpp`: habilitamos el nuevo puntero de atributo (glVertexAttribPointer / glEnableVertexAttribArray) especificando su location, la cantidad de componentes, el tipo de dato, el nuevo stride (tamaño total de Vertex) y el desplazamiento (offset).

```c++
//cambia solo esta funcion

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
```

Ahora en la parte de la gpu:

- `Vertex shader`: declaramos la nueva entrada layout `(location = 2) in vec2 aTexCoords`

Si un atributo se declara en el shader y no se usa, el compilador de GLSL puede eliminarlo.
¿Qué devuelve entonces una consulta por su ubicación, y por qué eso no es un error?
