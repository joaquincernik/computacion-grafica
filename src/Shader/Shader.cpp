#include "Shader.h"
#include <glad/gl.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using std::cerr;
using std::endl;
using std::string;
using std::vector;

static bool checkCompileErrors(GLuint shader, const std::string &type)
{
    const bool is_program = (type == "PROGRAM");
    GLint success = 0;
    if (is_program)
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
    }
    else
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    }

    if (!success)
    {
        GLint log_length = 0;
        if (is_program)
        {
            glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        }
        else
        {
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        }

        std::vector<char> info_log(log_length);
        if (is_program)
        {
            glGetProgramInfoLog(shader, log_length, nullptr, info_log.data());
        }
        else
        {
            glGetShaderInfoLog(shader, log_length, nullptr, info_log.data());
        }

        std::cerr << "[ERROR::SHADER::" << type << "::COMPILATION_FAILED]\n"
                  << info_log.data() << std::endl;

        return false; // Hubo un error
    }

    return true; // Compiló correctamente
}
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
bool Shader::compile_from_source(const string &vs, const string &fs)
{
    // Si ya había un shader compilado previamente, limpiamos
    clear();

    const char *vs_code = vs.c_str();
    const char *fs_code = fs.c_str();

    // 1. Crear y compilar el Vertex Shader
    GLuint vertex_shader = crear_vertex_shader_GPU(vs_code);
    if (!checkCompileErrors(vertex_shader, "VERTEX"))
    {
        glDeleteShader(vertex_shader);
        return false;
    }

    // 2. Crear y compilar el Fragment Shader
    GLuint fragment_shader = crear_fragment_shader_GPU(fs_code);
    if (!checkCompileErrors(fragment_shader, "FRAGMENT"))
    {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return false;
    }

    // 3. Crear el Program Object y vincular (link) los shaders
    GLint program = crear_program_GPU(vertex_shader, fragment_shader);

    // Verificar errores de linkeo
    if (!checkCompileErrors(program, "PROGRAM"))
    {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(program);
        return false;
    }

    // 4. Liberar los objetos de shader intermedios tras vincular exitosamente
    borrarObjetosIntermedios(vertex_shader, fragment_shader, program);

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

// nuevo practico 3
void Shader::set_uniform(const std::string &nombre, const glm::mat4 &m) const
{
    const int loc = glGetUniformLocation(id_, nombre.c_str());
    glProgramUniformMatrix4fv(id_, loc, 1, GL_FALSE, &m[0][0]); // DSA
}

GLuint Shader::crear_vertex_shader_GPU(const char *vs_code)
{
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vs_code, nullptr);
    glCompileShader(vertex_shader);
    return vertex_shader;
}

GLuint Shader::crear_fragment_shader_GPU(const char *fs_code)
{
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fs_code, nullptr);
    glCompileShader(fragment_shader);
    return fragment_shader;
}
GLuint Shader::crear_program_GPU(const GLuint vertex_shader, const GLuint fragment_shader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    return program;
}

void Shader::borrarObjetosIntermedios(const GLuint vertex_shader, const GLuint fragment_shader, const GLuint program)
{
    // le digo a la placa de video que borre los objetos intermedios
    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}