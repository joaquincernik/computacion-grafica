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
#include "CameraSystem/CameraSystem.h"

using glm::mat4;
using glm::vec3;
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

// Variables globales para el control de la cámara
static float g_yaw_delta = 0.0f;
static float g_pitch_delta = 0.0f;
static float g_dist_delta = 0.0f;
static float g_lastX = 400.0f;
static float g_lastY = 300.0f;
static bool g_firstMouse = true;
static const float sensitivity = 0.005f; // Sensibilidad en radianes
static const float zoom_speed = 0.1f;	 // Ajusta este valor si quieres que acerque más rápido o más lento

static int glfw_error_code{};
static string glfw_error_str{};

static GLFWwindow *crear_window();
static void fatal_error(const string &error_message, GLFWwindow *window = nullptr);
static void error_callback(int error, const char *description);
static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void processInput(GLFWwindow *window);
static void print_gl_version(void);
static void draw(const Shader &shader, const Mesh &mesh, const glm::mat4 &model, const glm::vec3 &color);
static void mouse_callback(GLFWwindow *window, double xpos, double ypos);

// Guardián del contexto de la ventana y GLFW
class WindowContext
{
public:
	GLFWwindow *window = nullptr;

	WindowContext(int width, int height, const char *title)
	{
		glfwSetErrorCallback(error_callback);
		if (!glfwInit())
		{
			fatal_error("GLFW initialization failed!");
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, kGLVerMajor);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, kGLVerMinor);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (!window)
		{
			fatal_error("Window creation failed!");
		}

		glfwMakeContextCurrent(window);

		if (!gladLoadGL(glfwGetProcAddress))
		{
			fatal_error("GLAD initialization failed!");
		}
	}

	~WindowContext()
	{
		if (window)
			glfwDestroyWindow(window);
		glfwTerminate();
	}

	// Evitamos copias accidentales
	WindowContext(const WindowContext &) = delete;
	WindowContext &operator=(const WindowContext &) = delete;
};
int main()
{
	// 1. El contexto se crea primero
	WindowContext ctx(kWindowWidth, kWindowHeight, kWindowTitle);

	print_gl_version();
	glfwSetFramebufferSizeCallback(ctx.window, framebuffer_size_callback);
	glfwSwapInterval(1);
	glfwSetCursorPosCallback(ctx.window, mouse_callback);
	glEnable(GL_DEPTH_TEST);

	// 2. Recursos OpenGL declarados al mismo nivel de scope
	Aircraft aircraft;
	aircraft.init();
	vector<RenderItem> items;

	ResourceManager resource_manager("shaders");
	const ShaderSource &shader_source = resource_manager.load_shader_source("escena", "shader.vert", "shader.frag");

	Shader shader_program;
	if (!shader_program.compile_from_source(shader_source.vs, shader_source.fs))
	{
		fatal_error("Shader compilation failed!", ctx.window);
	}

	CameraSystem camera_system(kWindowWidth, kWindowHeight);
	glfwSetWindowUserPointer(ctx.window, &camera_system);

	vec3 posicion(0.0f);
	vec3 angulos(-20.0f, -40.0f, 0.0f);

	// 3. Bucle principal limpio
	while (!glfwWindowShouldClose(ctx.window))
	{
		processInput(ctx.window);

		glClearColor(51.0f / 255.0f, 55.0f / 255.0f, 76.0f / 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		CameraCommand cmd{g_yaw_delta, g_pitch_delta, g_dist_delta};
		g_yaw_delta = g_pitch_delta = g_dist_delta = 0.0f;
		camera_system.update(posicion, angulos, cmd);

		shader_program.use();
		shader_program.set_uniform("uView", camera_system.data().view);
		shader_program.set_uniform("uProjection", camera_system.data().projection);

		items.clear();
		aircraft.update(posicion, angulos);
		aircraft.collect(items);

		for (const RenderItem &item : items)
		{
			draw(shader_program, *item.mesh, item.model, glm::vec3(item.color));
		}

		glfwSwapBuffers(ctx.window);
		glfwPollEvents();
	}

	return EXIT_SUCCESS;
	// Al salir de main(), primero se destruyen los recursos (aircraft, shaders),
	// y AL FINAL se ejecuta ~WindowContext(), llamando a glfwDestroyWindow y glfwTerminate en orden.
}
void error_callback(int error, const char *description)
{
	glfw_error_code = error;
	glfw_error_str = string(description);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
	// Recuperamos la cámara asociada a la ventana
	if (height > 0)
	{
		auto *camera = static_cast<CameraSystem *>(glfwGetWindowUserPointer(window));
		if (camera != nullptr)
		{
			camera->set_viewport(width, height);
		}
	}
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		g_dist_delta -= zoom_speed; // Acercar (reducir distancia)
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		g_dist_delta += zoom_speed; // Alejar (aumentar distancia)
	}
}

void print_gl_version(void)
{
	cout << " OpenGL Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << " OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << " OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << " GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

static GLFWwindow *crear_window()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, kGLVerMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, kGLVerMinor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	return glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle, nullptr, nullptr);
}
static void fatal_error(const string &error_message, GLFWwindow *window)
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

void draw(const Shader &shader, const Mesh &mesh, const glm::mat4 &model, const glm::vec3 &color)
{
	shader.use();
	shader.set_uniform("uModel", model);
	shader.set_uniform("uColor", color);
	glBindVertexArray(mesh.vao());
	glDrawElements(GL_TRIANGLES, mesh.count(), GL_UNSIGNED_INT, nullptr);
}
static void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	xpos = static_cast<float>(xpos);
	ypos = static_cast<float>(ypos);

	if (g_firstMouse)
	{
		g_lastX = xpos;
		g_lastY = ypos;
		g_firstMouse = false;
	}

	float xoffset = xpos - g_lastX;
	float yoffset = ypos - g_lastY;

	g_lastX = xpos;
	g_lastY = ypos;

	g_yaw_delta += xoffset * sensitivity;
	g_pitch_delta += yoffset * sensitivity;
}
