#pragma once
#include "CameraSystem/CameraCommand.h"
#include <GLFW/glfw3.h>
#include <dlfdm/defines.h> // Contiene la definición de dlfdm::ControlInputs

class InputHandler
{
public:
    InputHandler() = default;
    void init(GLFWwindow *window);
    void update(GLFWwindow *window, float dt); // polling, 1 vez por cuadro
                                               // Métodos para el FDM
    const dlfdm::ControlInputs &controls() const;
    void set_controls(const dlfdm::ControlInputs &c);
    CameraCommand camera_cmd() const; // del Practico 05

private:
    CameraCommand m_cmd{};
    void mouse_movement(double xpos, double ypos);
    void plane_movement(GLFWwindow *window, float dt); // Método para manejar el movimiento del avión
    dlfdm::ControlInputs m_controls{};

    // Estado del mouse
    float m_lastX = 400.0f;
    float m_lastY = 300.0f;
    bool m_firstMouse = true;

    // Parámetros de sensibilidad
    const float m_sensitivity = 0.005f; // Radianes por píxel
    const float m_zoomSpeed = 2.0f;     // Ajustado por delta time (
};