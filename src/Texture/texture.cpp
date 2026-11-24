#include "texture.h"
#include <glad/gl.h>
#include <algorithm> // std::max
#include <cmath>     // std::log2, std::floor

void Texture::clear(void)
{
    if (id_ != 0U)
        glDeleteTextures(1, &id_);
    id_ = 0U;
}

Texture::~Texture()
{
    clear();
}

Texture::Texture(Texture &&other) noexcept : id_(other.id_)
{
    other.id_ = 0U;
}

Texture &Texture::operator=(Texture &&other) noexcept
{
    if (this != &other) // protege de  t = std::move(t);
    {
        clear();         // libero la mía
        id_ = other.id_; // tomo la del otro
        other.id_ = 0U;  // el otro ya no es dueño
    }
    return *this;
}

void Texture::bind(unsigned int unit) const
{
    glBindTextureUnit(unit, id_); // DSA: reemplaza glActiveTexture + glBindTexture
}

void Texture::load_2d(const ImageData &image)
{
    clear(); // si ya tenía una textura, la libero antes de crear otra

    if (image.pixels.empty())
    {
        std::cerr << "[Texture ERROR] load_2d con imagen vacia" << std::endl;
        return;
    }
    // niveles de mipmap: floor(log2(max(w,h))) + 1  ->  256 px = 9 niveles
    int niveles = static_cast<int>(std::floor(std::log2(std::max(image.width, image.height)))) + 1;

    // 1. CREAR
    glCreateTextures(GL_TEXTURE_2D, 1, &id_);
    // 2. RESERVAR
    glTextureStorage2D(id_, niveles, GL_RGBA8, image.width, image.height);
    // 3. LLENAR
    glTextureSubImage2D(id_, 0, 0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels.data());
    //
    glGenerateTextureMipmap(id_);
    // 4. PARAMETRIZAR  (4 llamadas a glTextureParameteri)
    //
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Texture::load_cubemap(const std::array<const ImageData *, 6> &faces)
{
    clear(); // el almacenamiento no se reserva dos veces
    for (const ImageData *f : faces)
        if (f->pixels.empty())
        {
            std::cerr << "[Texture ERROR] cubemap con una cara vacia" << std::endl;
            return;
        }
    const int w = faces[0]->width, h = faces[0]->height;
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &id_);
    glTextureStorage2D(id_, 1, GL_RGB8, w, h); // se reserva en 2D...
    for (int cara = 0; cara < 6; ++cara)
        glTextureSubImage3D(id_, 0, 0, 0, cara, w, h, 1, // ...y se llena en 3D:

                            GL_RGBA, GL_UNSIGNED_BYTE, // la cara es la capa de la textura
                            faces[cara]->pixels.data());
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // en las tres direcciones
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}