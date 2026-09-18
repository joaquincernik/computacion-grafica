#include "Shader.h"
#include <glad/gl.h>
#include <iostream>
#include <vector>

// Destructor: destruye el programa en la GPU si existe
Shader::~Shader()
{
    clear();
}

// cuando creo un objeto a partir de uno existente,  el objeto todavia no existe en memoria
/*
Shader s1;
Shader s2 = std::move(s1); // <-- CONSTRUCTOR de movimiento
*/
Shader::Shader(Shader &&other) noexcept : id_(other.id_)
{
    other.id_ = 0U;
}

// Reemplazo el contenido que ya existe yu este puede tener asignado un recurso
/*
Shader s1;
Shader s2; // s2 ya fue creado previamente y tiene su propio shader en la GPU (ej: ID = 5)
s2 = std::move(s1); // <-- ASIGNACIÓN por movimiento
*/
Shader &Shader::operator=(Shader &&other) noexcept
{
    if (this != &other)
    {
        clear(); // Liberar nuestro programa actual si lo había

        id_ = other.id_;
        other.id_ = 0U;
    }
    return *this;
}

// Compila el Vertex y Fragment Shader, los linkea y limpia los shaders intermedios
bool Shader::compile_from_source(const std::string &vs, const std::string &fs)
{
    // Si ya había un shader compilado previamente, limpiamos
    clear();

    const char *vs_code = vs.c_str();
    const char *fs_code = fs.c_str();

    GLint success = 0;

    // 1. Crear y compilar el Vertex Shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vs_code, nullptr);
    glCompileShader(vertex_shader);

    // Verificar errores de compilación del Vertex Shader
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint log_length = 0;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> info_log(log_length);
        glGetShaderInfoLog(vertex_shader, log_length, nullptr, info_log.data());

        std::cerr << "[ERROR::SHADER::VERTEX::COMPILATION_FAILED]\n"
                  << info_log.data() << std::endl;

        glDeleteShader(vertex_shader);
        return false;
    }

    // 2. Crear y compilar el Fragment Shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fs_code, nullptr);
    glCompileShader(fragment_shader);

    // Verificar errores de compilación del Fragment Shader
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint log_length = 0;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> info_log(log_length);
        glGetShaderInfoLog(fragment_shader, log_length, nullptr, info_log.data());

        std::cerr << "[ERROR::SHADER::FRAGMENT::COMPILATION_FAILED]\n"
                  << info_log.data() << std::endl;

        // Limpieza de recursos intermedios creados hasta el momento
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return false;
    }

    // 3. Crear el Program Object y vincular (link) los shaders
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // Verificar errores de linkeo
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLint log_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> info_log(log_length);
        glGetProgramInfoLog(program, log_length, nullptr, info_log.data());

        std::cerr << "[ERROR::SHADER::PROGRAM::LINKING_FAILED]\n"
                  << info_log.data() << std::endl;

        // Limpieza total si falla el linkeo
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(program);
        return false;
    }

    // 4. Liberar los objetos de shader intermedios tras vincular exitosamente
    // le digo a la placa de video que borre los objetos intermedios
    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Guardar el identificador del programa exitoso
    id_ = program;
    return true;
}

// Activa el shader en el pipeline de OpenGL
void Shader::use(void) const
{
    if (id_ != 0U)
    {
        glUseProgram(id_);
    }
}

// Libera el recurso en la GPU
void Shader::clear(void)
{
    if (id_ != 0U)
    {
        glDeleteProgram(id_);
        id_ = 0U;
    }
}