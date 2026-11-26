#include "InputHandler.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <iostream>

const float ELEV_RANGO = glm::radians(5.0f);
const float ALERON_MAX = glm::radians(20.0f);
const float TIMON_MAX = glm::radians(8.0f);

const float K_PHI = 0.3f; // nivelador: cuánto alerón por radián de inclinación
const float K_P = 0.2f;   // nivelador: frena la velocidad de rolido para que no oscile
const float K_R = 0.5f;

static float s_scroll_delta = 0.0f;
using namespace std;

// Callback que GLFW llamará cada vez que gire la rueda
static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    (void)window; // Evita warnings de variable no usada
    (void)xoffset;
    s_scroll_delta += static_cast<float>(yoffset);
}

static float acercar(float valor, float objetivo, float paso)
{
    float diferencia = objetivo - valor;           // cuánto falta
    return valor + clamp(diferencia, -paso, paso); // avanza a lo sumo 'paso'
}

const dlfdm::ControlInputs &InputHandler::controls() const
{
    return m_controls;
}

void InputHandler::set_controls(const dlfdm::ControlInputs &c)
{
    m_controls = c;
    m_trim = c;
}

void InputHandler::init(GLFWwindow *window)
{
    glfwSetScrollCallback(window, scroll_callback);
}

CameraCommand InputHandler::camera_cmd() const
{
    return m_cmd;
}

void InputHandler::update(GLFWwindow *window, float dt, const dlfdm::AircraftState &estado)
{
    // Reseteamos deltas al inicio de cada frame

    m_cmd = CameraCommand{};

    // --- EXIT ---
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // --- PLANE MOVEMENT ---
    plane_movement(window, dt, estado); // Actualizamos los controles del avión según el teclado

    // --- SCROLL (ZOOM) ---
    m_cmd.dist_delta -= s_scroll_delta * m_zoomSpeed;
    s_scroll_delta = 0.0f; // Reseteamos el acumulador para el próximo cuadro

    // --- TOGGLE GAMMA ---
    bool tecla_i_actual = (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS);
    if (tecla_i_actual && !tecla_i_previa_)
    {
        usar_gamma_ = !usar_gamma_;
        std::cout << "Correccion Gamma: " << (usar_gamma_ ? "ON" : "OFF") << std::endl;
    }
    tecla_i_previa_ = tecla_i_actual;

    // --- POLLING DEL MOUSE ---
    double xpos_d, ypos_d;
    glfwGetCursorPos(window, &xpos_d, &ypos_d); // Leemos la posición actual
    mouse_movement(xpos_d, ypos_d);             // Calculamos el delta y actualizamos las variables globales
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
    {
        m_cmd.yaw_delta = 0.0f;
        m_cmd.pitch_delta = 0.0f;
    }
}

void InputHandler::plane_movement(GLFWwindow *window, float dt, const dlfdm::AircraftState &estado)
{
    float rate = 0.30f; // Velocidad de movimiento del mando

    // Elevador: de -1.0 a 1.0
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        m_controls.elevator += rate * dt;
        m_sin_alerones = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        m_controls.elevator -= rate * dt;
        m_sin_alerones = 0.0f;
    }
    else
        m_controls.elevator = acercar(m_controls.elevator, m_trim.elevator, rate * dt); // Volvemos al trim si no hay input

    m_controls.elevator = clamp(m_controls.elevator, m_trim.elevator - ELEV_RANGO, m_trim.elevator + ELEV_RANGO);

    // Potencia: de 0.0 a 1.0
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_controls.throttle += rate * dt;
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_controls.throttle -= rate * dt;
    else
        m_controls.throttle = acercar(m_controls.throttle, m_trim.throttle, rate * dt); // Volvemos al trim si no hay input

    m_controls.throttle = clamp(m_controls.throttle, 0.0f, 1.0f);

    // --- ALERONES (Bastón izquierda/derecha) ---
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        m_controls.aileron += rate * dt; // Ala derecha abajo (alabeo a la derecha)
        m_sin_alerones = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        m_controls.aileron -= rate * dt; // Alabeo a la izquierda
        m_sin_alerones = 0.0f;
    }
    else
    {
        m_sin_alerones += dt;
        if (m_sin_alerones > m_espera_nivelar)
            m_controls.aileron = -K_PHI * estado.phi - K_P * estado.body_omega.x; // Nivelador
        else
            m_controls.aileron = acercar(m_controls.aileron, m_trim.aileron, rate * dt); // Alerón a neutro, mantiene la inclinación
    }

    m_controls.aileron = clamp(m_controls.aileron, -ALERON_MAX, ALERON_MAX);
    // --- TIMÓN DE DIRECCIÓN (Pedales) ---
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_pedal -= rate * dt; // Pedal derecho -> timón negativo -> nariz a la derecha
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_pedal += rate * dt; // Pedal izquierdo -> timón positivo -> nariz a la izquierda
    else
        m_pedal = acercar(m_pedal, m_trim.rudder, rate * dt); // Volvemos al trim si no hay input

    m_pedal = clamp(m_pedal, -TIMON_MAX, TIMON_MAX);

    // Amortiguador de guiñada: frena el vaivén de la nariz (dutch roll)
    m_controls.rudder = m_pedal + K_R * estado.body_omega.z;
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
    float yoffset = ypos - m_lastY;

    m_lastX = xpos;
    m_lastY = ypos;

    m_cmd.yaw_delta = xoffset * m_sensitivity;
    m_cmd.pitch_delta = yoffset * m_sensitivity;
}
