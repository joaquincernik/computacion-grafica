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
#include "Scene/Aircraft/FlightData.h"
#include <dlfdm/models/aircraft/jettrainer.h>
#include <dlfdm/defines.h>
#include <dlfdm/fdmsolver.h>
#include "Primitives/Primitives.h"
#include "Utils/Alturas.h"
#include "Texture/texture.h"
#include <array>

using glm::inverse;
using glm::mat3;
using glm::mat4;
using glm::translate;
using glm::transpose;
using glm::vec3;
using std::cerr;
using std::cout;
using std::endl;
using std::min;
using std::string;
using std::vector;
using glm::vec4;

static const char *kWindowTitle = "OpenGL - Primitiva Cubo";
static constexpr int kWindowWidth = 800;
static constexpr int kWindowHeight = 600;
static constexpr int kGLVerMajor = 4;
static constexpr int kGLVerMinor = 6;

static string glfw_error_str{};
static void fatal_error(const string &error_message, GLFWwindow *window = nullptr);
static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void print_gl_version(void);
static void enable_wireframe_mode();
static void draw(const Shader &shader, const Mesh &mesh, const glm::mat4 &model, const glm::vec3 &color, const glm::vec3 &ks = vec3(0.0f), float brillo = 1.0f);
static void enable_fill_mode();
int main()
{
	// 1. El contexto se crea primero
	WindowContext ctx(kWindowWidth, kWindowHeight, kWindowTitle);

	print_gl_version();
	glfwSetFramebufferSizeCallback(ctx.window, framebuffer_size_callback);
	glfwSwapInterval(1);
	// glfwSetCursorPosCallback(ctx.window, mouse_callback); si queremos no polling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// -- AERONAVE --
	Aircraft aircraft;
	// aircraft.init();
	vector<RenderItem> items;

	// -- TERRENO --
	Alturas alturas(1337, 0.01f, 800.0f);
	int N = 400;
	float step = 50.0f;											   // 400 * 50.0m = 20.000m (20 km de extensión)
	float anchoTotal = N * step;								   // semilla, frecuencia, amplitud[cite: 2]
	MeshData terreno_data = primitives::terreno(N, step, alturas); // N=200, step=25.0m
	// 1. Crear el objeto Mesh vacío
	Mesh terreno_mesh;
	// 2. Cargar los datos a la GPU (crea el VAO, VBO y EBO)
	terreno_mesh.load(terreno_data);

	// -- SHADER --
	ResourceManager resource_manager(".");
	const ShaderSource &shader_source = resource_manager.load_shader_source("escena", "shaders/shader.vert", "shaders/shader.frag");
	Shader shader_program;
	if (!shader_program.compile_from_source(shader_source.vs, shader_source.fs))
		fatal_error("Shader compilation failed!", ctx.window);

	// --uniforms de ilumincacion
	shader_program.use();
	shader_program.set_uniform("uLuzPos", vec3(5000.0f, 15000.0f, -10000.0f)); // Posición (o dirección) del sol
	shader_program.set_uniform("uLuzColor", vec3(1.0f, 0.98f, 0.95f));		   // Color/intensidad de la luz
	shader_program.set_uniform("uAmbiente", vec3(0.25f, 0.25f, 0.30f));
	
	const ShaderSource &terreno_source = resource_manager.load_shader_source("terreno", "shaders/terreno.vert", "shaders/terreno.frag");
	Shader shader_terreno;
	if (!shader_terreno.compile_from_source(terreno_source.vs, terreno_source.fs))
		fatal_error("Terreno shader compilation failed!", ctx.window);

	const ShaderSource &fondo_source = resource_manager.load_shader_source("fondo", "shaders/background.vert", "shaders/background.frag");
	Shader shader_fondo;
	if (!shader_fondo.compile_from_source(fondo_source.vs, fondo_source.fs))
		fatal_error("Fondo shader compilation failed!", ctx.window);

	// -- TEXTURAS --
	const ImageData &img = resource_manager.load_image("suelo", "src/asssets/texturas/texturas-suelo/suelo/grass.png", true);
	Texture tex_suelo;
	tex_suelo.load_2d(img);

	const ImageData &img_prueba = resource_manager.load_image("prueba", "src/asssets/texturas/prueba.png", true);
	Texture tex_prueba;
	tex_prueba.load_2d(img_prueba);

	// -- AERONAVE: se arma acá porque init() necesita la textura --
	aircraft.init(&tex_prueba);

	
	// -- CÁMARA --
	CameraSystem camera_system(kWindowWidth, kWindowHeight);
	glfwSetWindowUserPointer(ctx.window, &camera_system);

	// -- INPUT HANDLER --
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
	mat4 model_terreno = translate(mat4(1.0f), vec3(-anchoTotal / 2.0f, 3500.0f, -anchoTotal));

	const std::string dir = "src/asssets/texturas/cielo-claros-y-nubes/";
	std::array<const ImageData *, 6> caras = {
		&resource_manager.load_image("cielo_right", dir + "right.png", false),
		&resource_manager.load_image("cielo_left", dir + "left.png", false),
		&resource_manager.load_image("cielo_top", dir + "top.png", false),
		&resource_manager.load_image("cielo_bottom", dir + "bottom.png", false),
		&resource_manager.load_image("cielo_front", dir + "front.png", false),
		&resource_manager.load_image("cielo_back", dir + "back.png", false),
	};
	Texture tex_cielo;
	tex_cielo.load_cubemap(caras);
	Mesh cubo_fondo;
	cubo_fondo.load(primitives::cube(1.0f));

	// u altura_suelo = 3500.0f; // altura del terreno en metros
	const float altura_terreno = 3500.0f;

	const float teselas = 400.0f;

	// loop
	while (!glfwWindowShouldClose(ctx.window))
	{
		double current_frame_time = static_cast<float>(glfwGetTime());
		double frame_dt = (float)(current_frame_time - last_frame_time);
		last_frame_time = current_frame_time;

		input_handler.update(ctx.window, frame_dt);
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

				// -- CIELO (método a: primero, sin escribir profundidad) --
		const mat4 vista_sin_traslacion = mat4(glm::mat3(camera_system.data().view));
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
		shader_fondo.use();
		shader_fondo.set_uniform("uProjection", camera_system.data().projection);
		shader_fondo.set_uniform("uView", vista_sin_traslacion);
		shader_fondo.set_uniform("uOjo", camera_system.data().eye);
		shader_fondo.set_uniform("uAlturaSuelo", altura_terreno);
		shader_fondo.set_uniform("uMetrosTesela", (N * step) / teselas);
		tex_cielo.bind(0);   // samplerCube uCielo        (binding = 0)
		tex_suelo.bind(1);   // sampler2D   uTexturaSuelo (binding = 1)
		glBindVertexArray(cubo_fondo.vao());

		shader_fondo.set_uniform("uOjo", camera_system.data().eye);// (C)
		glDrawElements(GL_TRIANGLES, cubo_fondo.count(), GL_UNSIGNED_INT, nullptr);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);

		
		shader_program.use();
		const mat4 &vista = camera_system.data().view;
		shader_program.set_uniform("uView", vista);
		shader_program.set_uniform("uProjection", camera_system.data().projection);
		shader_program.set_uniform("uLightDir", glm::normalize(vec3(0.4f, 1.0f, 0.3f)));

		// Posición del ojo extraída de la cuarta columna de la inversa de vista
		vec3 ojo = vec3(glm::inverse(vista)[3]);
		shader_program.set_uniform("uOjo", ojo);

		// -- -DIBUJAR TERRENO-- -
		shader_terreno.set_uniform("uView", camera_system.data().view);				// (A)
		shader_terreno.set_uniform("uProjection", camera_system.data().projection); // (A)
		shader_terreno.set_uniform("uTeselas", teselas);								// (B)
		tex_suelo.bind(0);			
		
		

		float medio_ancho = (N * step) / 2.0f; // (D)
		mat4 model_terreno = glm::translate(mat4(1.0f), vec3(-medio_ancho, altura_terreno, -medio_ancho));
		draw(shader_terreno, terreno_mesh, model_terreno, vec3(0.2f, 0.5f, 0.2f)); // (E)

		items.clear();
		aircraft.update(posicion, angulos);
		aircraft.collect(items);

		for (const RenderItem &item : items)
		{
			if (item.texture != nullptr)
			{
				item.texture->bind(0);						  // la letra en la unidad 0
				shader_terreno.set_uniform("uTeselas", 1.0f); // una sola vez por cara
				draw(shader_terreno, *item.mesh, item.model, glm::vec3(item.color));
			}
			else
			{
				draw(shader_program, *item.mesh, item.model, glm::vec3(item.color));
			}
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

void draw(const Shader &shader, const Mesh &mesh, const glm::mat4 &model, const glm::vec3 &color, const glm::vec3 &ks, float brillo)
{
	shader.use();

	// Transformaciones del objeto y su matriz normal
	shader.set_uniform("uModel", model);
	mat3 matNormal = glm::transpose(inverse(mat3(model)));
	shader.set_uniform("uMatNormal", matNormal);

	// Propiedades del material
	shader.set_uniform("uColor", color);
	shader.set_uniform("uKs", ks);
	shader.set_uniform("uBrillo", brillo);

	glBindVertexArray(mesh.vao());
	glDrawElements(GL_TRIANGLES, mesh.count(), GL_UNSIGNED_INT, nullptr);
}

// Activar modo de dibujo de líneas
void enable_wireframe_mode()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

// Activar modo de relleno de polígonos
void enable_fill_mode()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}