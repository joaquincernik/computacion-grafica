#include "ResourceManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include "Utils/stb_image.h"

using std::cerr;
using std::cout;
using std::endl;
using std::filesystem::exists;
using std::filesystem::path;
using std::ifstream;
using std::string;
using std::stringstream;

static const ImageData empty_image = {0, 0, 0, {}};

ResourceManager::ResourceManager(const path& assets_root) {
    assets_root_ = assets_root; // constructor, en .h solo declaramos que existe su conttructor
}

string ResourceManager::read_shader_file(const string& shader_file,
                                              const string& shader_type) {
    if (shader_file.empty()) {
        return "";
    }
    
    // Construimos la ruta combinando la raíz de assets con el archivo dado
    path full_path = assets_root_ / shader_file;

    // Validación: No rompe en silencio si no existe la carpeta o archivo
    if (!exists(full_path)) {
        cerr << "[ResourceManager ERROR] El archivo/carpeta no existe: " 
                  << full_path << " (" << shader_type << ")" << endl;
        return "";
    }

    ifstream file;
    stringstream stream;

    file.exceptions(ifstream::failbit | ifstream::badbit);
    try {
        file.open(full_path);
        stream << file.rdbuf();
        file.close();
    }
    catch (const ifstream::failure& e) {
        cerr << "[ResourceManager ERROR] Fallo al leer el archivo: " 
                  << full_path << " (" << shader_type << ")" << endl;
        return "";
    }

    return stream.str();
}

const ShaderSource& ResourceManager::load_shader_source(const string& key,
                                                         const string& vs_file,
                                                         const string& fs_file,
                                                         const string& gs_file,
                                                         const string& common_fs_file) {
    // CACHÉ: Si el recurso ya se cargó con esa clave, no vuelve a leer de disco
    auto it = shaders_sources_.find(key); //auto significa qeu deduzca el valor de la variable dependiendo de lo que se le asigna
    if (it != shaders_sources_.end()) {
        cout << "[ResourceManager INFO] Usando cache para la clave: " << key << endl;
        return it->second;
    }

    // 2. Si es la primera vez, leemos los archivos del disco
    ShaderSource source;
    source.vs = read_shader_file(vs_file, "VERTEX");
    string fs_code = read_shader_file(fs_file, "FRAGMENT");

    if (!common_fs_file.empty()) {
        string common_code = read_shader_file(common_fs_file, "FRAGMENT_COMMON");
        source.fs = common_code + "\n" + fs_code;
    } else {
        source.fs = fs_code;
    }
    
    if (!gs_file.empty()) {
        source.gs = read_shader_file(gs_file, "GEOMETRY");
    }

    // 3. Almacenamos en el unordered_map y retornamos la referencia retenida en memoria
    shaders_sources_[key] = source;
    return shaders_sources_[key];
}

const ShaderSource& ResourceManager::get_shader_source(const string& key) const {
    auto it = shaders_sources_.find(key);
    if (it == shaders_sources_.end()) {
        cerr << "[ResourceManager ERROR] No se encontro el recurso con la clave: " << key << endl;
        static const ShaderSource empty_source = {"", "", ""};
        return empty_source;
    }
    return it->second;
}

void ResourceManager::clear(void) {
    shaders_sources_.clear();
    images_.clear();
}

const ImageData& ResourceManager::load_image(const string& key, const string& file, bool flip_vertically) {
    // 1. Caché: buscar key en images_; si está, devolver it->second
    auto it = images_.find(key);
    if (it != images_.end()) {
        return it->second;
    }
    // 2. Armar full_path = assets_root_ / file y verificar exists() (como en read_shader_file)
    path full_path = assets_root_ / file;
    if (!exists(full_path)) {
        cerr << "[ResourceManager ERROR] El archivo de imagen no existe: " << full_path << endl;
        return empty_image;
    }
    // 3. stbi_set_flip_vertically_on_load(flip_vertically);   // es GLOBAL: fijarlo siempre
    // dar vuelta vertical a la imagen al cargarla, porque OpenGL tiene el origen en la esquina inferior izquierda,
    // mientras que la mayoría de las imágenes tienen el origen en la esquina superior izquierda. Esto asegura que 
    // la imagen se cargue correctamente para su uso en OpenGL.
    stbi_set_flip_vertically_on_load(flip_vertically); 
    // 4. Decodificar:
    int w, h, canales;
    unsigned char* datos = stbi_load(full_path.string().c_str(), &w, &h, &canales, 4);
    if (datos == nullptr) {
        cerr << "[ResourceManager ERROR] Error al cargar la imagen: " << stbi_failure_reason() << endl;
        return empty_image;
    }

    // 5. Llenar un ImageData: width, height, channels, y
    ImageData img;
    img.width    = w;
    img.height   = h;
    img.channels = canales;
    img.pixels.assign(datos, datos + w * h * 4);
    // 6. stb reservó con malloc, lo liberás vos
    stbi_image_free(datos);
    // 7. Guardar en images_[key] y devolver images_[key]
    images_[key] = std::move(img);
    return images_[key];
}

const ImageData& ResourceManager::get_image(const string& key) const {
    // igual que get_shader_source: find, y si no está -> cerr + static const ImageData vacía
    auto it = images_.find(key);
    if (it == images_.end()) {
        cerr << "[ResourceManager ERROR] No se encontro el recurso con la clave: " << key << endl;
        static const ImageData empty_image = {0, 0, 0, {}};
        return empty_image;
    }
    return it->second;
}