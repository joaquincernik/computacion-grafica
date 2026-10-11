#include "InputHandler.h"
#include <algorithm>

static float s_scroll_delta = 0.0f;
using namespace std;

// Callback que GLFW llamará cada vez que gires la rueda
static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    (void)window; // Evita warnings de variable no usada
    (void)xoffset;
    s_scroll_delta += static_cast<float>(yoffset);
}

const dlfdm::ControlInputs &InputHandler::controls() const
{
    return m_controls;
}

void InputHandler::set_controls(const dlfdm::ControlInputs &c)
{
    m_controls = c;
}

void InputHandler::init(GLFWwindow *window)
{
    glfwSetScrollCallback(window, scroll_callback);
}

void InputHandler::update(GLFWwindow *window, float dt)
{
    // Reseteamos deltas al inicio de cada frame
    float rate = 1.0f; // Velocidad de movimiento del mando
    m_cmd = CameraCommand{};

    // --- TECLADO ---
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // Elevador: de -1.0 a 1.0
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        m_controls.elevator += rate * dt;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        m_controls.elevator -= rate * dt;
    m_controls.elevator = clamp(m_controls.elevator, -1.0f, 1.0f);

    // Potencia: de 0.0 a 1.0
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_controls.throttle += rate * dt;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_controls.throttle -= rate * dt;
    m_controls.throttle = clamp(m_controls.throttle, 0.0f, 1.0f);

    // --- ALERONES (Bastón izquierda/derecha) ---
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        m_controls.aileron += rate * dt; // Ala derecha abajo (alabeo a la derecha)
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        m_controls.aileron -= rate * dt; // Alabeo a la izquierda
    }
    m_controls.aileron = clamp(m_controls.aileron, -1.0f, 1.0f);

    // --- TIMÓN DE DIRECCIÓN (Pedales) ---
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        m_controls.rudder -= rate * dt; // Pedal derecho -> timón negativo -> nariz a la derecha
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        m_controls.rudder += rate * dt; // Pedal izquierdo -> timón positivo -> nariz a la izquierda
    }
    m_controls.rudder = clamp(m_controls.rudder, -1.0f, 1.0f);

    // --- SCROLL (ZOOM) ---
    // Consumimos el acumulador de scroll (hacia adelante yoffset es positivo)
    // No lo multiplicamos por dt porque el scroll es un evento discreto, no continuo.
    m_cmd.dist_delta -= s_scroll_delta * m_zoomSpeed;
    s_scroll_delta = 0.0f; // Reseteamos el acumulador para el próximo cuadro

    // --- POLLING DEL MOUSE ---
    double xpos_d, ypos_d;
    glfwGetCursorPos(window, &xpos_d, &ypos_d); // Leemos la posición actual
    mouse_movement(xpos_d, ypos_d);             // Calculamos el delta y actualizamos las variables globales
}

CameraCommand InputHandler::camera_cmd() const
{
    return m_cmd;
}

void InputHandler::mouse_movement(double xpos, double ypos)
{
    xpos = static_cast<float>(xpos);
    ypos = static_cast<float>(ypos);
    if (m_firstMouse)
    {
        m_lastX = xpos;
        m_lastY = ypos;
        m_firstMouse = false;
    }

    float xoffset = xpos - m_lastX;
    float yoffset = ypos - m_lastY; // Invertimos el eje Y

    m_lastX = xpos;
    m_lastY = ypos;

    m_cmd.yaw_delta = xoffset * m_sensitivity;
    m_cmd.pitch_delta = yoffset * m_sensitivity;
}