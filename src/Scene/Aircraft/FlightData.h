#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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
