#pragma once
#include "CameraSystem/CameraCommand.h"
#include <GLFW/glfw3.h>
#include <dlfdm/defines.h> // Contiene la definición de dlfdm::ControlInputs

class InputHandler
{
public:
    InputHandler() = default;
    void init(GLFWwindow *window);
    void update(GLFWwindow *window, float dt, const dlfdm::AircraftState &estado); // polling, 1 vez por cuadro
                                                                                   // Métodos para el FDM
    const dlfdm::ControlInputs &controls() const;
    void set_controls(const dlfdm::ControlInputs &c);
    CameraCommand camera_cmd() const; // del Practico 05
    bool usar_gamma() const { return usar_gamma_; }


private:
    CameraCommand m_cmd{};
    void mouse_movement(double xpos, double ypos);
    void plane_movement(GLFWwindow *window, float dt, const dlfdm::AircraftState &estado); // Método para manejar el movimiento del avión
    dlfdm::ControlInputs m_controls{};
    dlfdm::ControlInputs m_trim{};

    // Estado del mouse
    float m_lastX = 400.0f;
    float m_lastY = 300.0f;
    bool m_firstMouse = true;

    // Parámetros de sensibilidad
    const float m_sensitivity = 0.005f; // Radianes por píxel
    const float m_zoomSpeed = 2.0f;     // Ajustado por delta time (

    // parametros de tiempo para estabilizacion del avion
    float m_sin_alerones = 0.0f;         // [s] tiempo sin tocar las flechas izq/der
    const float m_espera_nivelar = 2.0f; // [s] cuánto espera antes de nivelar solo


    // iluminacion gamma
    bool usar_gamma_ = true;
    bool tecla_i_previa_ = false;


    float m_pedal = 0.0f;        // rebote de los pedales, para que no se quede en el trim
    bool m_pedal_activo = false; // el timon estaba deflectado el cuadro anterior
    float m_tope_pedal = 0.0f;
};