# Practico 1

Se creo la carpeta de Resource manager adaptando la propuesta de la consigna a la clase de resource manager.

Originalmente el codigo lo realizaba usando esta funcion

```c++

// a dibujar ese triangulo chad
// Función para leer archivos de texto (shaders) desde el disco
static std::string loadShaderSource(const char *filePath)
{
	std::ifstream file;
	std::stringstream stream;

	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		file.open(filePath);
		stream << file.rdbuf();
		file.close();
	}
	catch (std::ifstream::failure &e)
	{
		std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << filePath << std::endl;
		return "";
	}
	return stream.str();
}
```

y lo llamaba aqui

```c++
std::string vertCode = loadShaderSource("shaders/shader.vert");
	std::string fragCode = loadShaderSource("shaders/shader.frag");

	const char *vertexShaderSource = vertCode.c_str();
	const char *fragmentShaderSource = fragCode.c_str();
//...
```

## Cuestiones a pensar

Respondamos las preguntas que nos plantea el practico:

- ¿El cache se indexa por una clave que elige el usuario del módulo, o directamente por el nombre del archivo?
  El cache se indexa depentiendo de la `key` que el usuario del modulo le pasa aqui:

```c++
	const ShaderSource &shader =
		resource_manager.load_shader_source("triangulo", "shader.vert", "shader.frag");
```

En este caso la key sera triangulo

- Si se pide usar una clave que ya está cargada pero con archivos distintos: ¿se recarga, se ignora lo nuevo, es un error?
  El concepto de cache funciona usar una key para ese mismo objeto, por simplicidad, si se llama a una key que esta almacenada se devuelve lo que esta almacenada.
  Esto podria mejorarse pero es mas facil simplemente crear una nueva key al estilo triangulo_nuevo, no considero que sea un error

- Cómo se avisa que algo falló: excepción, valor de retorno u otra cosa?
  Se realiza este chequeo en el `Resource Manager`

```c++
if (!std::filesystem::exists(full_path)) {
        std::cerr << "[ResourceManager ERROR] El archivo/carpeta no existe: "
                  << full_path << " (" << shader_type << ")" << std::endl;
        return "";
    }
```

Esto devuelve un string vacio y el main.cpp se encarga de cortar la ejecucion ante cualquier problema

```c++
if (shader.vs.empty() || shader.fs.empty())
	{
		std::cerr << "ERROR::SHADER::SOURCES_INCOMPLETOS" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return EXIT_FAILURE;
	}
```

- ¿Devuelve una referencia const o una copia? ¿Qué pasa con esa referencia si alguien llama a clear() después?
  Devuelve una referencia constante (const ShaderSource&)  
  Riesgo con clear(): Si el usuario llama a clear(), los datos dentro del mapa se eliminan, lo que invalida todas las referencias obtenidas previamente. Cualquier intento de leer esa referencia después del clear() causará un error de memoria

## Prueba de cache

Creamos el archivo de cache_test el cual hara las pruebas para detectar si en diferentes llamadas se usa el mismo valor guardado en el mapa

```c++
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
```

Lo compilamos de la siguiente manera, ubicados en la raiz del proyecto

```bash
g++ -std=c++17 -Wall -Wextra src/ResourceManager/ResourceManager.cpp src/cache_test.cpp -o test
```

Lo corremos y obtenemos lo siguiente:

```bash
 ./test
=== INICIANDO PRUEBAS DE RESOURCE MANAGER ===

[Prueba 1] Cargando shader por primera vez...
Vertex Shader leido (108 bytes):
# version 460 core
layout ( location = 0) in vec3 aPos ;
void main ()
{
gl_Position = vec4 ( aPos , 1.0) ;
}
[Prueba 2] Solicitando el mismo shader por segunda vez...
[ResourceManager INFO] Usando cache para la clave: mi_shader
0x567ee6b297f8 == 0x567ee6b297f8 --> aca vemos claramente que esta usando el mismo archivo
>> SUCCESS: El shader fue recuperado del CACHE (misma direccion de memoria).

[Prueba 3] Recuperando clave existente con get_shader_source...
>> SUCCESS: Obtenido desde get_shader_source con exito.

[Prueba 4] Intentando cargar un archivo inexistente...
[ResourceManager ERROR] El archivo/carpeta no existe: "./shaders/no_existe.vert" (VERTEX)
[ResourceManager ERROR] El archivo/carpeta no existe: "./shaders/no_existe.frag" (FRAGMENT)

=== PRUEBAS FINALIZADAS EXITOSAMENTE ===
```

Vemos que funciono correctamente
