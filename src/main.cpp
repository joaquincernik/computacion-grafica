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
#include "Utils/WindowContext.h"
#include "InputHandler/InputHandler.h"
#include <dlfdm/models/aircraft/jettrainer.h>
#include <dlfdm/defines.h>
#include <dlfdm/fdmsolver.h>

using glm::mat4;
using glm::vec3;
using std::cerr;
using std::cout;
using std::endl;
using std::min;
using std::string;
using std::vector;

static const char *kWindowTitle = "OpenGL - Primitiva Cubo";
static constexpr int kWindowWidth = 800;
static constexpr int kWindowHeight = 600;
static constexpr int kGLVerMajor = 4;
static constexpr int kGLVerMinor = 6;

// Variables globales para el control de la cámara
static float g_pitch_delta = 0.0f;
static float g_yaw_delta = 0.0f;
static float g_dist_delta = 0.0f;
static float g_lastX = 400.0f;
static float g_lastY = 300.0f;
static bool g_firstMouse = true;
static const float sensitivity = 0.005f; // Sensibilidad en radianes
static const float zoom_speed = 0.1f;	 // Ajusta este valor si quieres que acerque más rápido o más lento

static string glfw_error_str{};
static void fatal_error(const string &error_message, GLFWwindow *window = nullptr);
static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void print_gl_version(void);
static void draw(const Shader &shader, const Mesh &mesh, const glm::mat4 &model, const glm::vec3 &color);

// Estructura y función para traducir de NED (Física) a OpenGL (Gráficos)
struct FlightData
{
	glm::vec3 position;
	float phi, theta, psi;
};

FlightData to_world(const dlfdm::AircraftState &ned)
{
	FlightData gl;
	gl.position.x = ned.inertial_position.y;
	gl.position.y = -ned.inertial_position.z;
	gl.position.z = -ned.inertial_position.x;
	gl.phi = -ned.phi;
	gl.theta = ned.theta;
	gl.psi = -ned.psi;
	return gl;
}

int main()
{
	// 1. El contexto se crea primero
	WindowContext ctx(kWindowWidth, kWindowHeight, kWindowTitle);

	print_gl_version();
	glfwSetFramebufferSizeCallback(ctx.window, framebuffer_size_callback);
	glfwSwapInterval(1);
	// glfwSetCursorPosCallback(ctx.window, mouse_callback); si queremos no polling
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

	InputHandler input_handler;		// aca metemos el input handler
	input_handler.init(ctx.window); // en este init le pasamos como callback

	// --- INICIALIZACIÓN DEL FDM ---
	dlfdm::AircraftParameters aermacchi_s211 = dlfdm::jettrainer::load_model();
	dlfdm::FDMSolver fdm(aermacchi_s211);

	dlfdm::TrimPoint trim = dlfdm::jettrainer::get_trim_condition(dlfdm::jettrainer::TrimCondition::kISA5000TAS150);
	fdm.setState(trim.state);
	input_handler.set_controls(trim.controls);
	// ------------------------------

	vec3 posicion(0.0f);
	vec3 angulos(-20.0f, -40.0f, 0.0f);

	const double dt = 1.0 / 120.0;
	float last_frame_time = static_cast<float>(glfwGetTime());
	double acc = 0.0;

	// 3. Bucle principal limpio
	while (!glfwWindowShouldClose(ctx.window))
	{
		double current_frame_time = static_cast<float>(glfwGetTime());
		double frame_dt = current_frame_time - last_frame_time;
		last_frame_time = current_frame_time;

		input_handler.update(ctx.window, (float)frame_dt);
		CameraCommand cmd = input_handler.camera_cmd();

		// acumulamso tope de tiempo
		acc += min(frame_dt, 0.25);
		while (acc >= dt)
		{
			// EL FDM AVANZA UN PASO FÍSICO
			fdm.update(input_handler.controls());
			acc -= dt;
		}

		// --- APLICAR LA FÍSICA A LA ESCENA ---
		FlightData datos_vuelo = to_world(fdm.getState());
		posicion = datos_vuelo.position;
		angulos = vec3(datos_vuelo.phi, datos_vuelo.theta, datos_vuelo.psi);
		// -------------------------------------

		camera_system.update(posicion, angulos, cmd);

		glClearColor(51.0f / 255.0f, 55.0f / 255.0f, 76.0f / 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

void print_gl_version(void)
{
	cout << " OpenGL Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << " OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << " OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << " GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
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