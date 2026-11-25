#pragma once
#include <array>
#include "ResourceManager/ResourceManager.h"

class Texture
{
public:
    Texture() = default;
    ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;
    // Los cuatro pasos (crear, reservar, llenar, parametrizar) para una
    // textura 2D, a partir de una imagen ya decodificada.
    void load_2d(const ImageData& image);
    // Lo mismo para un cubemap: seis caras en el orden +X -X +Y -Y +Z -Z.
    void load_cubemap(const std::array<const ImageData*, 6>& faces);
    // Conecta la textura a una unidad de textura para el proximo dibujo.
    void bind(unsigned int unit) const;
    void clear(void);
    unsigned int id(void) const { return id_; }
private:
    unsigned int id_ {0U};
};