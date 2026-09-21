#include <cstdlib> // EXIT_FAILURE, EXIT_SUCCESS
#include <iostream>
#include <string>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ResourceManager/ResourceManager.h"
#include "Mesh/Mesh.h"
#include "Primitives/Primitives.h"
#include "Shader/Shader.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

static const char *kWindowTitle = "OpenGL - Primitiva Cubo";
static constexpr int kWindowWidth = 800;
static constexpr int kWindowHeight = 600;
static constexpr int kGLVerMajor = 4;
static constexpr int kGLVerMinor = 6;

static int glfw_error_code{};
static string glfw_error_str{};

static void error_callback(int error, const char *description);
static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void processInput(GLFWwindow *window);
static void print_gl_version(void);
static void draw(const Shader &shader, const Mesh &mesh, const glm::mat4 &model, const glm::vec3 &color);
static glm::mat4 armar_modelo(const glm::vec3 &translation, float angulo, const glm::vec3 &rotation, const glm::vec3 &scale);
static GLFWwindow *crear_window()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, kGLVerMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, kGLVerMinor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	return glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle, nullptr, nullptr);
}
static void fatal_error(const string &error_message, GLFWwindow *window = nullptr)
{
	cerr << error_message << endl;

	// Si se pasó una ventana válida, destruirla
	if (window != nullptr)
	{
		glfwDestroyWindow(window);
	}

	// Terminar GLFW de forma segura
	glfwTerminate();

	// Forzar la salida del programa
	exit(EXIT_FAILURE);
}

int main()
{
	// Registrar callback de errores antes de la inicialización
	glfwSetErrorCallback(error_callback);

	// 1. Inicializar GLFW
	if (!glfwInit())
		fatal_error("GLFW initialization failed! - " + std::to_string(glfw_error_code) + ": " + glfw_error_str);

	//------configuracion de window
	GLFWwindow *window = crear_window();
	if (window == nullptr)
		fatal_error("GLFW initialization failed! - " + std::to_string(glfw_error_code) + ": " + glfw_error_str);

	glfwMakeContextCurrent(window); // hace que el contexto OpenGL de la ventana sea el "actual" en el hilo actual. Es obligatorio antes de llamar a cualquier función OpenGL.
	//---------------------------------------------------

	// 2. Cargar punteros de OpenGL mediante GLAD (es un loader de punteros de funciones de opengl)
	if (!gladLoadGL(glfwGetProcAddress))
		fatal_error("GLFW initialization failed! - " + std::to_string(glfw_error_code) + ": " + glfw_error_str);

	print_gl_version();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSwapInterval(1);

	// Habilitar el test de profundidad para el renderizado 3D correcto
	glEnable(GL_DEPTH_TEST);

	// ETAPA 1: Decidir los datos (Generar datos del cubo con el módulo Primitives)
	// MeshData cube_data = primitives::cube(1.0f, 1.0f, 1.0f);
	MeshData cube_data, cylinder_data, cone_data;
	cube_data = primitives::cube(1.0f, 1.0f, 1.0f);
	cylinder_data = primitives::cylinder(0.4f, 1.2f, 32);
	cone_data = primitives::cone(0.5f, glm::radians(50.0f), 32);

	// ETAPA 2: Pasar los datos a la GPU (Cargar la malla indexada en la GPU)
	Mesh cube_mesh, cylinder_mesh, cone_mesh;
	cube_mesh.load(cube_data);
	cylinder_mesh.load(cylinder_data);
	cone_mesh.load(cone_data);

	// ETAPA 3: Armar el programa de Shaders (Leer fuentes y compilar/linkear)
	ResourceManager resource_manager("shaders");
	const ShaderSource &shader_source = resource_manager.load_shader_source("escena", "shader.vert", "shader.frag");

	if (shader_source.vs.empty() || shader_source.fs.empty())
	{
		cube_mesh.clear();
		cylinder_mesh.clear();
		cone_mesh.clear();
		fatal_error("ERROR::SHADER::SOURCES_INCOMPLETOS", window);
	}

	Shader shader_program;
	if (!shader_program.compile_from_source(shader_source.vs, shader_source.fs))
	{
		cube_mesh.clear();
		cylinder_mesh.clear();
		cone_mesh.clear();
		shader_program.clear();
		fatal_error("ERROR::SHADER::COMPILACION_O_LINKEO_FALLIDO", window);
	}

	// matriz de ajuste
	const float ancho = 800.0f, alto = 600.0f; // de la ventana
	const glm::mat4 ajuste = glm::scale(glm::mat4(1.0f), glm::vec3(alto / ancho, 1.0f, -1.0f));
	shader_program.use();
	shader_program.set_uniform("uAjuste", ajuste); // invierte eje z para la regla de mano derecha y ajsuta window

	glm::mat4 cube_model = armar_modelo(glm::vec3(-0.9f, 0.0f, 0.0f), 30.0f, glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.75f));
	glm::mat4 cylinder_model = armar_modelo(glm::vec3(0.0f, 0.0f, 0.0f), 25.0f, glm::vec3(1.0f, 0.5f, 0.0f), glm::vec3(0.5f, 0.8f, 0.5f));
	glm::mat4 cone_model = armar_modelo(glm::vec3(0.9f, 0.0f, 0.0f), 30.0f, glm::vec3(0.8f, 1.0f, 0.0f), glm::vec3(0.6f));

	// Bucle principal de renderizado
	while (!glfwWindowShouldClose(window))
	{
		processInput(window); // lee si el usuario no puso esc por ejemplo

		// Limpiar el búfer de color y el búfer de profundidad
		glClearColor(51.0f / 255.0f, 55.0f / 255.0f, 76.0f / 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ETAPA 4: Dibujar
		/*shader_program.use();
		glBindVertexArray(cube_mesh.vao()); // enlazamos con el vao

		// Dibujo con índices utilizando la cantidad de índices de la malla
		glDrawElements(GL_TRIANGLES, cube_mesh.count(), GL_UNSIGNED_INT, nullptr);
*/
		draw(shader_program, cube_mesh, cube_model, glm::vec3(0.9f, 0.2f, 0.2f));
		draw(shader_program, cone_mesh, cone_model, glm::vec3(0.2f, 0.8f, 0.3f));
		draw(shader_program, cylinder_mesh, cylinder_model, glm::vec3(0.2f, 0.5f, 1.0f));

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Liberar los recursos de OpenGL mientras el contexto siga activo
	cube_mesh.clear();
	cone_mesh.clear();
	cylinder_mesh.clear();
	shader_program.clear();

	// Limpieza final de la ventalt textana y GLFW tras la destrucción de los objetos OpenGL
	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}

void error_callback(int error, const char *description)
{
	glfw_error_code = error;
	glfw_error_str = string(description);
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
	cout << " OpenGL Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << " OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << " OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << " GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

glm::mat4 armar_modelo(const glm::vec3 &translation, float angulo, const glm::vec3 &rotation, const glm::vec3 &scale)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, translation);

	if (angulo != 0.0f)
	{
		model = glm::rotate(model, glm::radians(angulo), rotation);
	}
	model = glm::scale(model, scale);
	return model;
}

void draw(const Shader &shader, const Mesh &mesh, const glm::mat4 &model, const glm::vec3 &color)
{
	shader.use();
	shader.set_uniform("uModel", model);
	shader.set_uniform("uColor", color);
	glBindVertexArray(mesh.vao());
	glDrawElements(GL_TRIANGLES, mesh.count(), GL_UNSIGNED_INT, nullptr);
}