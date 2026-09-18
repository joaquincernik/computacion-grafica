#include <iostream>
#include <cassert>
#include "ResourceManager/ResourceManager.h"

int main()
{
    std::cout << "=== INICIANDO PRUEBAS DE RESOURCE MANAGER ===" << std::endl;

    // 1. Instanciar el ResourceManager apuntando al directorio raíz o de assets
    ResourceManager manager("./");

    // 2. Primera carga (debe leer desde el disco)
    std::cout << "\n[Prueba 1] Cargando shader por primera vez..." << std::endl;
    const ShaderSource &shader1 = manager.load_shader_source(
        "mi_shader",
        "shaders/shader.vert",
        "shaders/shader.frag");

    std::cout << "Vertex Shader leido (" << shader1.vs.size() << " bytes):\n"
              << shader1.vs << std::endl;

    // 3. Segunda carga con la misma clave (DEBE USAR EL CACHÉ)
    std::cout << "[Prueba 2] Solicitando el mismo shader por segunda vez..." << std::endl;
    const ShaderSource &shader2 = manager.load_shader_source(
        "mi_shader",
        "shaders/shader.vert",
        "shaders/shader.frag");

    std::cout << &shader1 << " == " << &shader2 << std::endl; //aca vemos efectivamente el mismo valor
    // Verificamos que las direcciones de memoria sean exactamente las mismas
    // (Demuestra que devolvió una referencia al mapa en memoria y no creó una copia ni releyó)
    if (&shader1 == &shader2)
    {
        std::cout << ">> SUCCESS: El shader fue recuperado del CACHE (misma direccion de memoria)." << std::endl;
    }
    else
    {
        std::cerr << ">> ERROR: Se creo una copia o se releyo el archivo." << std::endl;
    }

    // 4. Búsqueda con get_shader_source
    std::cout << "\n[Prueba 3] Recuperando clave existente con get_shader_source..." << std::endl;
    const ShaderSource &shader_get = manager.get_shader_source("mi_shader");
    assert(&shader1 == &shader_get);
    std::cout << ">> SUCCESS: Obtenido desde get_shader_source con exito." << std::endl;

    // 5. Prueba de manejo de errores (archivo inexistente)
    std::cout << "\n[Prueba 4] Intentando cargar un archivo inexistente..." << std::endl;
    manager.load_shader_source("shader_invalido", "shaders/no_existe.vert", "shaders/no_existe.frag");

    std::cout << "\n=== PRUEBAS FINALIZADAS EXITOSAMENTE ===" << std::endl;
    return 0;
}