#include <cstdlib> // EXIT_FAILURE, EXIT_SUCCESS
#include <iostream>
#include <string>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ResourceManager/ResourceManager.h"
#include "Mesh/Mesh.h"
#include "Material/Material.h"
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
using glm::vec4;
using std::array;
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

static string glfw_error_str{};
static void fatal_error(const string &error_message, GLFWwindow *window = nullptr);
static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void print_gl_version(void);
static void draw(const Shader &shader, const Mesh &mesh, const glm::mat4 &model, const Material &material);
static void enable_wireframe_mode();
static void enable_fill_mode();
static array<const ImageData *, 6> cargar_caras_cubemap(ResourceManager &rm, const string &directorio, const string &prefijo_clave = "cielo", const string &extension = ".png");
// static void enviar_entorno(const Shader &shader, const glm::vec3 &ojo);
static void configurar_entorno_fijo(const Shader &s);

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
	aircraft.init();
	vector<RenderItem> items;

	// -- TERRENO --
	Alturas alturas(1337, 0.01f, 800.0f);
	int N = 400;
	float step = 50.0f;											   // 400 * 50.0m = 20.000m (20 km de extensión)
	float anchoTotal = N * step;								   // semilla, frecuencia, amplitud[cite: 2]
	MeshData terreno_data = primitives::terreno(N, step, alturas); // N=200, step=25.0m
	Mesh terreno_mesh;
	terreno_mesh.load(terreno_data);

	// -- SHADER --
	ResourceManager resource_shaders_manager("shaders");
	// escena shader
	const ShaderSource &shader_source = resource_shaders_manager.load_shader_source("escena", "shader.vert", "shader.frag", "", "iluminacion.glsl");
	Shader shader_program;
	if (!shader_program.compile_from_source(shader_source.vs, shader_source.fs))
		fatal_error("Shader compilation failed!", ctx.window);
	configurar_entorno_fijo(shader_program);

	// terreno shader
	const ShaderSource &terreno_source = resource_shaders_manager.load_shader_source("terreno", "terreno.vert", "terreno.frag", "", "iluminacion.glsl");
	Shader shader_terreno;
	if (!shader_terreno.compile_from_source(terreno_source.vs, terreno_source.fs))
		fatal_error("Terreno shader compilation failed!", ctx.window);
	configurar_entorno_fijo(shader_terreno);

	// background shader
	const ShaderSource &fondo_source = resource_shaders_manager.load_shader_source("fondo", "background.vert", "background.frag", "", "iluminacion.glsl");
	Shader shader_fondo;
	if (!shader_fondo.compile_from_source(fondo_source.vs, fondo_source.fs))
		fatal_error("Fondo shader compilation failed!", ctx.window);
	configurar_entorno_fijo(shader_fondo);

	// -- TEXTURAS --
	ResourceManager resource_textures_manager("src/asssets/texturas");
	const ImageData &img = resource_textures_manager.load_image("suelo", "texturas-suelo/suelo/grass.png", true);
	Texture tex_suelo;
	tex_suelo.load_2d(img);

	// armamos cielo
	auto caras = cargar_caras_cubemap(resource_textures_manager, "cielo-claros-y-nubes/");
	Texture tex_cielo;
	tex_cielo.load_cubemap(caras);
	Mesh cubo_fondo;
	cubo_fondo.load(primitives::cube(1.0f));

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

	// -- PARAMETROS DE ESCENA --
	const float altura_terreno = 3500.0f;
	const float teselas = 400.0f;
	float medio_ancho = (N * step) / 2.0f; // (D)

	const double dt = 1.0 / 120.0;
	float last_frame_time = static_cast<float>(glfwGetTime());
	double acc = 0.0;
	mat4 model_terreno = translate(mat4(1.0f), vec3(-medio_ancho, altura_terreno, -medio_ancho));
	// loop
	while (!glfwWindowShouldClose(ctx.window))
	{
		double current_frame_time = static_cast<float>(glfwGetTime());
		double frame_dt = (float)(current_frame_time - last_frame_time);
		last_frame_time = current_frame_time;

		input_handler.update(ctx.window, frame_dt, fdm.getState());
		static double proximo_print = 0.0;
		if (current_frame_time >= proximo_print)
		{
			const dlfdm::AircraftState &s = fdm.getState();
			cout << "V: " << (int)glm::length(s.body_velocity) << " m/s"
				 << "  Alt: " << (int)-s.inertial_position.z << " m"
				 << "  Pot: " << (int)(input_handler.controls().throttle * 100) << "%" << endl;
			proximo_print = current_frame_time + 0.5;
		}

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

		camera_system.update(posicion, angulos, cmd, static_cast<float>(frame_dt));

		glClearColor(51.0f / 255.0f, 55.0f / 255.0f, 76.0f / 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const vec3 &ojo = camera_system.data().eye;
		bool gamma_activo = input_handler.usar_gamma() ? true : false;

		// -- CIELO (método a: primero, sin escribir profundidad) --
		const mat4 vista_sin_traslacion = mat4(mat3(camera_system.data().view));
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
		shader_fondo.use();
		shader_fondo.set_uniform("uProjection", camera_system.data().projection);
		shader_fondo.set_uniform("uView", vista_sin_traslacion);
		shader_fondo.set_uniform("uOjo", ojo);
		shader_fondo.set_uniform("uAlturaSuelo", altura_terreno);
		shader_fondo.set_uniform("uMetrosTesela", (N * step) / teselas);
		shader_fondo.set_uniform("uUsarGamma", gamma_activo);
		tex_cielo.bind(0); // samplerCube uCielo        (binding = 0)
		tex_suelo.bind(1); // sampler2D   uTexturaSuelo (binding = 1)
		glBindVertexArray(cubo_fondo.vao());
		glDrawElements(GL_TRIANGLES, cubo_fondo.count(), GL_UNSIGNED_INT, nullptr);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);

		// shader escena
		shader_program.use();
		const mat4 &vista = camera_system.data().view;
		shader_program.set_uniform("uView", vista);
		shader_program.set_uniform("uProjection", camera_system.data().projection);
		shader_program.set_uniform("uOjo", ojo);
		shader_program.set_uniform("uUsarGamma", gamma_activo);

		// -- -DIBUJAR TERRENO-- -
		shader_terreno.use();
		shader_terreno.set_uniform("uView", camera_system.data().view); // (A)
		shader_terreno.set_uniform("uOjo", ojo);
		shader_terreno.set_uniform("uProjection", camera_system.data().projection); // (A)
		shader_terreno.set_uniform("uTeselas", teselas);							// (B)
		shader_terreno.set_uniform("uUsarGamma", gamma_activo);
		tex_suelo.bind(0);

		Material mat_terreno{glm::vec3(1.0f), glm::vec3(0.0f), 1.0f};
		draw(shader_terreno, terreno_mesh, model_terreno, mat_terreno); // (E)

		items.clear();
		aircraft.update(posicion, angulos);
		aircraft.collect(items);

		for (const RenderItem &item : items)
		{
			if (item.texture != nullptr)
			{
				item.texture->bind(0);						  // la letra en la unidad 0
				shader_terreno.set_uniform("uTeselas", 1.0f); // una sola vez por cara
				draw(shader_terreno, *item.mesh, item.model, item.material);
			}
			else
			{
				draw(shader_program, *item.mesh, item.model, item.material);
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

void draw(const Shader &shader, const Mesh &mesh, const mat4 &model, const Material &material)
{
	shader.use();

	// Transformaciones del objeto y su matriz normal
	shader.set_uniform("uModel", model);
	mat3 matNormal = glm::transpose(inverse(mat3(model)));
	shader.set_uniform("uMatNormal", matNormal);

	// Propiedades del material (struct Material en iluminacion.glsl)
	shader.set_uniform("uMaterial.kd", material.kd);
	shader.set_uniform("uMaterial.ks", material.ks);
	shader.set_uniform("uMaterial.ns", material.ns);

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

// esto es principalmente para que no quede esto en el main
/*const std::string dir = "src/asssets/texturas/cielo-claros-y-nubes/";
	std::array<const ImageData *, 6> caras = {
		&resource_manager.load_image("cielo_right", dir + "right.png", false),
		&resource_manager.load_image("cielo_left", dir + "left.png", false),
		&resource_manager.load_image("cielo_top", dir + "top.png", false),
		&resource_manager.load_image("cielo_bottom", dir + "bottom.png", false),
		&resource_manager.load_image("cielo_front", dir + "front.png", false),
		&resource_manager.load_image("cielo_back", dir + "back.png", false),
	};*/
array<const ImageData *, 6> cargar_caras_cubemap(
	ResourceManager &rm,
	const string &directorio,
	const string &prefijo_clave,
	const string &extension)
{
	static const array<const char *, 6> nombres = {
		"right", "left", "top", "bottom", "front", "back"};

	array<const ImageData *, 6> caras{};
	for (size_t i = 0; i < 6; ++i)
	{
		string clave = prefijo_clave + "_" + nombres[i];
		string ruta = directorio + nombres[i] + extension;
		caras[i] = &rm.load_image(clave, ruta, false);
	}
	return caras;
}

/* si se hace de noche si conviene enviarle todo
static void enviar_entorno(const Shader &shader, const vec3 &ojo)
{
	shader.use();
	shader.set_uniform("uSolDir", normalize(vec3(0.4f, 1.0f, 0.3f)));
	shader.set_uniform("uSolColor", vec3(1.0f, 0.98f, 0.95f));
	shader.set_uniform("uAmbiente", vec3(0.25f, 0.25f, 0.30f));
	shader.set_uniform("uOjo", ojo);
	shader.set_uniform("uNieblaColor", vec3(0.65f, 0.75f, 0.88f)); // color del horizonte
	shader.set_uniform("uVisibilidad", 8000.0f); // distancia en metros (~63% niebla)
}
*/
void configurar_entorno_fijo(const Shader &s)
{
	s.use();
	s.set_uniform("uSolDir", normalize(vec3(-0.7f, 0.65f, -0.3f)));
	s.set_uniform("uSolColor", vec3(0.80f, 0.78f, 0.75f));
	s.set_uniform("uAmbiente", vec3(0.20f, 0.20f, 0.22f));
	s.set_uniform("uNieblaColor", vec3(0.55f, 0.68f, 0.85f));
	s.set_uniform("uVisibilidad", 8000.0f);
}
