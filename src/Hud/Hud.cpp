#include "Hud.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Shader/Shader.h"
#include "Utils/stb_easy_font.h"

Hud::~Hud()
{
    // glDelete* ignora los ids en 0, no hace falta chequear si init() se llamo
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
}

void Hud::init()
{
    buffer_.resize(kMaxQuads * kBytesPorQuad);

    // indices fijos: cada quad son 4 vertices seguidos -> 2 triangulos (0,1,2) y (0,2,3)
    std::vector<unsigned int> indices;
    indices.reserve(kMaxQuads * 6);
    for (unsigned int k = 0; k < kMaxQuads; ++k)
    {
        const unsigned int b = 4 * k;
        indices.insert(indices.end(), {b, b + 1, b + 2, b, b + 2, b + 3});
    }

    glCreateVertexArrays(1, &vao_);
    glCreateBuffers(1, &vbo_);
    glCreateBuffers(1, &ebo_);

    // el vbo se reserva vacio y se rellena en cada draw; el ebo no cambia nunca
    glNamedBufferData(vbo_, buffer_.size(), nullptr, GL_DYNAMIC_DRAW);
    glNamedBufferData(ebo_, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // stride 16: x,y,z (12 bytes) + color RGBA (4 bytes) que salteamos, el color va por uniform
    glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, kBytesPorQuad / 4);
    glVertexArrayElementBuffer(vao_, ebo_);
    glVertexArrayAttribFormat(vao_, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao_, 0, 0);
    glEnableVertexArrayAttrib(vao_, 0);
}

void Hud::draw(const Shader &shader, const char *texto, float x, float y, float escala,
               const glm::vec3 &color, int ancho, int alto)
{
    // stb pide char* pero no modifica el texto; dibujamos en (0,0) y movemos con uModel
    const int quads = stb_easy_font_print(0.0f, 0.0f, const_cast<char *>(texto), nullptr,
                                          buffer_.data(), static_cast<int>(buffer_.size()));
    glNamedBufferSubData(vbo_, 0, quads * kBytesPorQuad, buffer_.data());

    const glm::mat4 proj = glm::ortho(0.0f, float(ancho), float(alto), 0.0f); // y hacia abajo, como stb
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(escala, escala, 1.0f));

    shader.use();
    shader.set_uniform("uProjection", proj);
    shader.set_uniform("uModel", model);
    shader.set_uniform("uColor", color);
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, quads * 6, GL_UNSIGNED_INT, nullptr);
}