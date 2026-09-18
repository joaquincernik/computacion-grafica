#include "ResourceManager.h"

ResourceManager::ResourceManager(const std::filesystem::path& assets_root) {
    assets_root_ = assets_root; // constructor, en .h solo declaramos que existe su conttructor
}

std::string ResourceManager::read_shader_file(const std::string& shader_file,
                                              const std::string& shader_type) {
    if (shader_file.empty()) {
        return "";
    }

    // Construimos la ruta combinando la raíz de assets con el archivo dado
    std::filesystem::path full_path = assets_root_ / shader_file;

    // Validación: No rompe en silencio si no existe la carpeta o archivo
    if (!std::filesystem::exists(full_path)) {
        std::cerr << "[ResourceManager ERROR] El archivo/carpeta no existe: " 
                  << full_path << " (" << shader_type << ")" << std::endl;
        return "";
    }

    std::ifstream file;
    std::stringstream stream;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file.open(full_path);
        stream << file.rdbuf();
        file.close();
    }
    catch (const std::ifstream::failure& e) {
        std::cerr << "[ResourceManager ERROR] Fallo al leer el archivo: " 
                  << full_path << " (" << shader_type << ")" << std::endl;
        return "";
    }

    return stream.str();
}

const ShaderSource& ResourceManager::load_shader_source(const std::string& key,
                                                         const std::string& vs_file,
                                                         const std::string& fs_file,
                                                         const std::string& gs_file) {
    // CACHÉ: Si el recurso ya se cargó con esa clave, no vuelve a leer de disco
    auto it = shaders_sources_.find(key); //auto significa qeu deduzca el valor de la variable dependiendo de lo que se le asigna
    if (it != shaders_sources_.end()) {
        std::cout << "[ResourceManager INFO] Usando cache para la clave: " << key << std::endl;
        return it->second;
    }

    // 2. Si es la primera vez, leemos los archivos del disco
    ShaderSource source;
    source.vs = read_shader_file(vs_file, "VERTEX");
    source.fs = read_shader_file(fs_file, "FRAGMENT");
    
    if (!gs_file.empty()) {
        source.gs = read_shader_file(gs_file, "GEOMETRY");
    }

    // 3. Almacenamos en el unordered_map y retornamos la referencia retenida en memoria
    shaders_sources_[key] = source;
    return shaders_sources_[key];
}

const ShaderSource& ResourceManager::get_shader_source(const std::string& key) const {
    auto it = shaders_sources_.find(key);
    if (it == shaders_sources_.end()) {
        std::cerr << "[ResourceManager ERROR] No se encontro el recurso con la clave: " << key << std::endl;
        static const ShaderSource empty_source = {"", "", ""};
        return empty_source;
    }
    return it->second;
}

void ResourceManager::clear(void) {
    shaders_sources_.clear();
}