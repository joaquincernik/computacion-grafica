#pragma once

#include <vector>
#include <glad/gl.h>
#include <glm/glm.hpp>

class Shader;

class Hud
{
public:
    Hud() = default;
    ~Hud(); // libera VAO/VBO/EBO
    Hud(const Hud &) = delete;            // dueño unico de los buffers, como Mesh
    Hud &operator=(const Hud &) = delete;

    void init(); // crea VAO/VBO y el EBO fijo de quads -> triangulos, UNA VEZ

    // texto en pixeles: (x, y) esquina superior izquierda, y crece hacia abajo
    void draw(const Shader &shader, const char *texto, float x, float y, float escala,
              const glm::vec3 &color, int ancho, int alto);

private:
    static constexpr int kMaxQuads = 1000;     // tope de quads por llamada (sobra para un renglon)
    static constexpr int kBytesPorQuad = 64;   // 4 vertices x 16 bytes (x,y,z float + color RGBA)

    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    std::vector<char> buffer_;                 // lo llena stb_easy_font_print cada cuadro
};