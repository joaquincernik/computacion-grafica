// ResourceManager.h --- punto de partida, no molde
#pragma once // rvita que el archivo se lea mas de una vez si lo incluyo en varios lugares
#include <string>
#include <filesystem>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
struct ShaderSource
{                   // codigo fuente de un programa de shader
    std::string vs; // vertex shader
    std::string fs; // fragment shader
    std::string gs; // geometry shader (opcional, hoy no se usa)
};
class ResourceManager
{
public:
    explicit ResourceManager(const std::filesystem::path &assets_root); // constructor de la clase que toma la carpeta raiz
    const ShaderSource &load_shader_source(const std::string &key,
                                           const std::string &vs_file,
                                           const std::string &fs_file,
                                           const std::string &gs_file = ""); // lee los archovos y los asocia a un id key. devuelve una referencia constante
    const ShaderSource &get_shader_source(const std::string &key) const;     // referebcia al shader usando su ley
    void clear(void);

private:
    std::filesystem::path assets_root_;                             // ruta base
    std::unordered_map<std::string, ShaderSource> shaders_sources_; // seria como el cache, mantiene relacion de clave, ej "triangulo" con sus shaders cargadis
    std::string read_shader_file(const std::string &shader_file,
                                 const std::string &shader_type);
};