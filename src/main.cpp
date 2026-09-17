#include <cstdlib> // EXIT_FAILURE, EXIT_SUCCESS
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

static const char *kWindowTitle = "OpenGL template project";
static constexpr int kWindowWidth = 800;
static constexpr int kWindowHeight = 600;
static constexpr int kGLVerMajor = 4;
static constexpr int kGLVerMinor = 6;

static int glfw_error_code{};
static std::string glfw_error_str{};

static void error_callback(int error, const char *description);
static void framebuffer_size_callback(GLFWwindow *window,
									  int width, int height);
static void processInput(GLFWwindow *window);
static void print_gl_version(void);

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
int main()
{

	// Set error callback before initialization so we can be notified
	glfwSetErrorCallback(error_callback);

	// Init GLFW
	if (!glfwInit())
	{
		const std::string error_msg = "GLFW initialization failed!";
		const std::string glfw_error_msg = std::to_string(glfw_error_code) +
										   "): " + glfw_error_str;
		std::cout << error_msg + " - GLFW(" + glfw_error_msg << std::endl;
		return EXIT_FAILURE;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, kGLVerMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, kGLVerMinor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(kWindowWidth,
										  kWindowHeight,
										  kWindowTitle, nullptr, nullptr);

	if (window == nullptr)
	{
		// Clean resources already created
		glfwTerminate();

		const std::string error_msg = "GLFW window creation failed!";
		const std::string glfw_error_msg = std::to_string(glfw_error_code) +
										   "): " + glfw_error_str;
		std::cout << error_msg + " - GLFW(" + glfw_error_msg << std::endl;

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	// Load GLAD pointers in the current GLFW context
	if (!gladLoadGL(glfwGetProcAddress))
	{
		// Clean resources already created
		glfwDestroyWindow(window);
		glfwTerminate();

		std::cout << "GLAD initialization failed!" << std::endl;

		return EXIT_FAILURE;
	}

	// Show current system OpenGL info
	print_gl_version();

	// Set GLFW callbacks
	// glViewport(0, 0, kWindowWidth, kWindowHeight);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Configure sync with monitor
	// interval: the number of screen updates to wait from the time
	// glfwSwapBuffers was called before swapping the buffers and returning.
	// 0 -> run at code speed (don't sync to monitor)
	// 1 -> sync with motinor refresh rate (code run at monitor refresh rate)
	// 2 -> sync with half monitor refresh rate (code run at half monitor
	//      refresh rate)
	glfwSwapInterval(1);

	//-==================== Aca empiezo ==================
	// SHADERS ---------------------
	// Leemos los archivos
	std::string vertCode = loadShaderSource("shaders/shader.vert");
	std::string fragCode = loadShaderSource("shaders/shader.frag");

	const char *vertexShaderSource = vertCode.c_str();
	const char *fragmentShaderSource = fragCode.c_str();

	// Compilar Vertex Shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Compilar Fragment Shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Linkear Programa
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Liberar instancias individuales
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//=============== Ahora si lo primero que vimos en clase =========================

	const float vertices[] = {
		0.0f, 0.5f, 0.0f,	// arriba
		-0.5f, -0.5f, 0.0f, // abajo izquierda
		0.5f, -0.5f, 0.0f	// abajo derecha
	};
	// hasta aca exister solo en memoria, hay que mandarselo a la gpu

	GLuint vbo = 0;
	glCreateBuffers(1, &vbo); // la GPU devuelve un nombre : un entero
	glNamedBufferData(vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// ahora copie los bytes crudos pero tengo que decirl como leerlos

	GLuint vao = 0;
	glCreateVertexArrays(1, &vao);
	glVertexArrayVertexBuffer(vao, 0, vbo, 0, 3 * sizeof(float));
	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao, 0, 0);
	glEnableVertexArrayAttrib(vao, 0);
	// declaracion del VAO, ya se como leerlos

	glEnableVertexArrayAttrib(vao, 0); // arranca apagado, hay que prenderlo


	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window);

		// rendering commands here
		glClearColor(51.0f / 256, 55.0f / 256, 76.0f / 256, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//dibujamods
		glUseProgram(shaderProgram);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Clean resources before close
	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}

void error_callback(int error, const char *description)
{
	glfw_error_code = error;
	glfw_error_str = std::string(description);
}

void framebuffer_size_callback([[maybe_unused]] GLFWwindow *window,
							   int width, int height)
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
	// show gl info
	std::cout << " OpenGL Vendor: "
			  << glGetString(GL_VENDOR) << std::endl;
	std::cout << " OpenGL Renderer: "
			  << glGetString(GL_RENDERER) << std::endl;
	std::cout << " OpenGL Version: "
			  << glGetString(GL_VERSION) << std::endl;
	std::cout << " GLSL Version: "
			  << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}
