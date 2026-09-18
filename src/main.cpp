#include <cstdlib> // EXIT_FAILURE, EXIT_SUCCESS
#include <iostream>
#include <string>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "ResourceManager/ResourceManager.h"
#include "Mesh/Mesh.h"
#include "Primitives/Primitives.h"
#include "Shader/Shader.h"

static const char *kWindowTitle = "OpenGL - Primitiva Cubo";
static constexpr int kWindowWidth = 800;
static constexpr int kWindowHeight = 600;
static constexpr int kGLVerMajor = 4;
static constexpr int kGLVerMinor = 6;

static int glfw_error_code{};
static std::string glfw_error_str{};

static void error_callback(int error, const char *description);
static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void processInput(GLFWwindow *window);
static void print_gl_version(void);

int main()
{
	// Registrar callback de errores antes de la inicialización
	glfwSetErrorCallback(error_callback);

	// 1. Inicializar GLFW
	if (!glfwInit())
	{
		std::cerr << "GLFW initialization failed! - " << glfw_error_code << ": " << glfw_error_str << std::endl;
		return EXIT_FAILURE;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, kGLVerMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, kGLVerMinor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle, nullptr, nullptr);

	if (window == nullptr)
	{
		glfwTerminate();
		std::cerr << "GLFW window creation failed! - " << glfw_error_code << ": " << glfw_error_str << std::endl;
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	// 2. Cargar punteros de OpenGL mediante GLAD
	if (!gladLoadGL(glfwGetProcAddress))
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		std::cerr << "GLAD initialization failed!" << std::endl;
		return EXIT_FAILURE;
	}

	print_gl_version();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSwapInterval(1);

	// Habilitar el test de profundidad para el renderizado 3D correcto
	glEnable(GL_DEPTH_TEST);

	// =========================================================================
	// ÁMBITO LIMITADO PARA RECURSOS DE OPENGL
	// Encapsulamos Mesh y Shader dentro de este bloque { } para asegurar que sus
	// destructores se ejecuten MIENTRAS el contexto de OpenGL siga activo.
	// =========================================================================
	{
		// ETAPA 1: Decidir los datos (Generar datos del cubo con el módulo Primitives)
		MeshData cube_data = primitives::cube(1.0f, 1.0f, 1.0f);

		// ETAPA 2: Pasar los datos a la GPU (Cargar la malla indexada en la GPU)
		Mesh cube_mesh;
		cube_mesh.load(cube_data);

		// ETAPA 3: Armar el programa de Shaders (Leer fuentes y compilar/linkear)
		ResourceManager resource_manager("shaders");
		const ShaderSource &shader_source = resource_manager.load_shader_source("triangulo", "shader.vert", "shader.frag");

		if (shader_source.vs.empty() || shader_source.fs.empty())
		{
			std::cerr << "ERROR::SHADER::SOURCES_INCOMPLETOS" << std::endl;
			glfwDestroyWindow(window);
			glfwTerminate();
			return EXIT_FAILURE;
		}

		Shader shader_program;
		if (!shader_program.compile_from_source(shader_source.vs, shader_source.fs))
		{
			std::cerr << "ERROR::SHADER::COMPILACION_O_LINKEO_FALLIDO" << std::endl;
			glfwDestroyWindow(window);
			glfwTerminate();
			return EXIT_FAILURE;
		}

		// Bucle principal de renderizado
		while (!glfwWindowShouldClose(window))
		{
			processInput(window);

			// Limpiar el búfer de color y el búfer de profundidad
			glClearColor(51.0f / 255.0f, 55.0f / 255.0f, 76.0f / 255.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// ETAPA 4: Dibujar
			shader_program.use();
			glBindVertexArray(cube_mesh.vao());
			
			// Dibujo con índices utilizando la cantidad de índices de la malla
			glDrawElements(GL_TRIANGLES, cube_mesh.count(), GL_UNSIGNED_INT, nullptr);

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	} // <- AQUÍ mueren `cube_mesh` y `shader_program`, liberando sus VBO/VAO/EBO/Program en la GPU.

	// Limpieza final de la ventana y GLFW tras la destrucción de los objetos OpenGL
	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}

void error_callback(int error, const char *description)
{
	glfw_error_code = error;
	glfw_error_str = std::string(description);
}

void framebuffer_size_callback([[maybe_unused]] GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void print_gl_version(void)
{
	std::cout << " OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << " OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << " OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << " GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}