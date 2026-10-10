#include <cstdlib> // EXIT_FAILURE, EXIT_SUCCESS
#include <iostream>
#include <string>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ResourceManager/ResourceManager.h"
#include "Mesh/Mesh.h"
#include "Shader/Shader.h"
#include "Scene/Aircraft/Aircraft.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

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

	// Los recursos OpenGL (mallas y shader) viven en este scope para que sus
	// destructores los liberen MIENTRAS el contexto sigue activo, antes de
	// glfwDestroyWindow/glfwTerminate.
	{
		// ETAPA 1 y 2: Generar las piezas de la aeronave y subirlas a la GPU
		Aircraft aircraft;
		aircraft.init();
		vector<RenderItem> items;

		// ETAPA 3: Armar el programa de Shaders (Leer fuentes y compilar/linkear)
		ResourceManager resource_manager("shaders");
		const ShaderSource &shader_source = resource_manager.load_shader_source("escena", "shader.vert", "shader.frag");

		if (shader_source.vs.empty() || shader_source.fs.empty())
		{
			fatal_error("ERROR::SHADER::SOURCES_INCOMPLETOS", window);
		}

		Shader shader_program;
		if (!shader_program.compile_from_source(shader_source.vs, shader_source.fs))
		{
			shader_program.clear();
			fatal_error("ERROR::SHADER::COMPILACION_O_LINKEO_FALLIDO", window);
		}

		// matriz de ajuste
		const float ancho = 800.0f, alto = 600.0f;
		const float escala = 0.8f; // Reduce todo el modelo al 50% para que no se recorte al rotar en 3D
		const glm::mat4 ajuste = glm::scale(glm::mat4(1.0f), glm::vec3((alto / ancho) * escala, escala, escala));
		shader_program.use();
		shader_program.set_uniform("uAjuste", ajuste); // invierte eje z para la regla de mano derecha y ajsuta window

		// Posición y orientación inicial del avión
		glm::vec3 posicion = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 angulos  = glm::vec3(-20.0f, -40.0f, 0.0f);
		// Bucle principal de renderizado
		while (!glfwWindowShouldClose(window))
		{
			processInput(window); // lee si el usuario no puso esc por ejemplo

			// Limpiar el búfer de color y el búfer de profundidad
			glClearColor(51.0f / 255.0f, 55.0f / 255.0f, 76.0f / 255.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// 1. Limpiar los elementos del frame anterior
			items.clear();

			// 2. Actualizar la matriz global del avión con la posición y ángulos
			aircraft.update(posicion, angulos);

			// 3. Llenar el vector 'items' con las 7 piezas transformadas
			aircraft.collect(items);

			// 4. Dibujar cada pieza recolectada
			for (const RenderItem &item : items)
			{
				draw(shader_program, *item.mesh, item.model, glm::vec3(item.color));
			}

			glfwSwapBuffers(window);
			glfwPollEvents();
		}

	} // fin del scope de recursos OpenGL: aca se destruyen mallas y shader

	// Limpieza final de la ventana y GLFW tras la destrucción de los objetos OpenGL
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

void draw(const Shader &shader, const Mesh &mesh, const glm::mat4 &model, const glm::vec3 &color)
{
	shader.use();
	shader.set_uniform("uModel", model);
	shader.set_uniform("uColor", color);
	glBindVertexArray(mesh.vao());
	glDrawElements(GL_TRIANGLES, mesh.count(), GL_UNSIGNED_INT, nullptr);
}