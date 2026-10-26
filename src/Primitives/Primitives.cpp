#include "Primitives.h"
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "Utils/Alturas.h"
#include <algorithm>
namespace primitives
{

    MeshData cube(float scale_x, float scale_y, float scale_z)
    {
        MeshData data;

        // Mitad de las dimensiones para centrar el cubo en el origen (0,0,0)
        const float hx = scale_x * 0.5f;
        const float hy = scale_y * 0.5f;
        const float hz = scale_z * 0.5f;

        data.vertices.reserve(24);
        data.indices.reserve(36);

        // Mantiene el orden antihorario mirando desde afuera para cada cara.
        data.vertices = {
            // CARA FRONTAL (+Z) -> Normal apuntando hacia +Z: {0.0f, 0.0f, 1.0f}
            {{-hx, -hy, hz}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // Inf-Izq (0)
            {{hx, -hy, hz}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},  // Inf-Der (1)
            {{hx, hy, hz}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // Sup-Der (2)
            {{-hx, hy, hz}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},  // Sup-Izq (3)

            // CARA TRASERA (-Z) -> Normal apuntando hacia -Z: {0.0f, 0.0f, -1.0f}
            {{hx, -hy, -hz}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},  // Inf-Izq (4)
            {{-hx, -hy, -hz}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}}, // Inf-Der (5)
            {{-hx, hy, -hz}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},  // Sup-Der (6)
            {{hx, hy, -hz}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},   // Sup-Izq (7)

            // CARA IZQUIERDA (-X) -> Normal apuntando hacia -X: {-1.0f, 0.0f, 0.0f}
            {{-hx, -hy, -hz}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // Inf-Izq (8)
            {{-hx, -hy, hz}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},  // Inf-Der (9)
            {{-hx, hy, hz}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},   // Sup-Der (10)
            {{-hx, hy, -hz}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},  // Sup-Izq (11)

            // CARA DERECHA (+X) -> Normal apuntando hacia +X: {1.0f, 0.0f, 0.0f}
            {{hx, -hy, hz}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},  // Inf-Izq (12)
            {{hx, -hy, -hz}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // Inf-Der (13)
            {{hx, hy, -hz}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},  // Sup-Der (14)
            {{hx, hy, hz}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},   // Sup-Izq (15)

            // CARA SUPERIOR (+Y) -> Normal apuntando hacia +Y: {0.0f, 1.0f, 0.0f}
            {{-hx, hy, hz}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},  // Inf-Izq (16)
            {{hx, hy, hz}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},   // Inf-Der (17)
            {{hx, hy, -hz}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},  // Sup-Der (18)
            {{-hx, hy, -hz}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // Sup-Izq (19)

            // CARA INFERIOR (-Y) -> Normal apuntando hacia -Y: {0.0f, -1.0f, 0.0f}
            {{-hx, -hy, -hz}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // Inf-Izq (20)
            {{hx, -hy, -hz}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},  // Inf-Der (21)
            {{hx, -hy, hz}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},   // Sup-Der (22)
            {{-hx, -hy, hz}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}   // Sup-Izq (23)
        };

        // Los índices no cambian
        data.indices = {
            0, 1, 2, 2, 3, 0,       // Cara Frontal
            4, 5, 6, 6, 7, 4,       // Cara Trasera
            8, 9, 10, 10, 11, 8,    // Cara Izquierda
            12, 13, 14, 14, 15, 12, // Cara Derecha
            16, 17, 18, 18, 19, 16, // Cara Superior
            20, 21, 22, 22, 23, 20  // Cara Inferior
        };

        return data;
    }
    MeshData cylinder(float radio, float largo, unsigned gajos, unsigned anillos)
    {
        MeshData data;
        const float PI = glm::pi<float>();

        // reserva de memoria
        const unsigned total_vertices = (gajos + 1) * (anillos + 1);
        const unsigned total_indices = gajos * anillos * 6;
        data.vertices.reserve(total_vertices);
        data.indices.reserve(total_indices);

        // generacion por vertices
        for (unsigned ring = 0; ring <= anillos; ++ring)
        {
            const float nivel_anillo = static_cast<float>(ring) / static_cast<float>(anillos); // si es el anillo superior o inferior
            const float y = nivel_anillo * largo;
            for (unsigned i = 0; i <= gajos; ++i)
            {
                const float th = 2.0f * PI * float(i) / float(gajos);
                Vertex v;
                v.position = {radio * cos(th), y, radio * sin(th)};
                v.normal = {cos(th), 0.0f, sin(th)};
                v.tex_coords = {float(i) / float(gajos), nivel_anillo};
                data.vertices.push_back(v);
            }
        }

        // indices
        for (unsigned i = 0; i < gajos; ++i)
        {
            const unsigned b0 = i, b1 = i + 1;                         // anillo de abajo
            const unsigned t0 = gajos + 1 + i, t1 = gajos + 1 + i + 1; // anillo de arriba
            data.indices.insert(data.indices.end(), {b0, t1, b1});
            data.indices.insert(data.indices.end(), {b0, t0, t1});
        }

        return data;
    }

    MeshData cone(float radio, float conicidad, unsigned gajos, unsigned anillos)
    {
        MeshData data;

        const unsigned N = gajos;
        const float PI = glm::pi<float>();

        // Si la conicidad viene en grados, convertir a radianes:
        // const float alpha = glm::radians(conicidad);
        const float alpha = conicidad;
        const float alpha_2 = alpha * 0.5f; // Semiángulo de conicidad (alpha/2)

        // Altura h calculada por trigonometría: tan(alpha/2) = radio / altura
        const float altura = radio / std::tan(alpha_2);

        // Reservamos memoria suficiente
        const unsigned total_vertices = (N + 1) * (anillos + 1);
        const unsigned total_indices = N * anillos * 6;
        data.vertices.reserve(total_vertices);
        data.indices.reserve(total_indices);

        // Precalculamos seno y coseno del semiángulo para las normales
        const float cos_alpha_2 = std::cos(alpha_2);
        const float sin_alpha_2 = std::sin(alpha_2);

        // 1. GENERACIÓN DE VÉRTICES POR ANILLOS
        for (unsigned ring = 0; ring <= anillos; ++ring)
        {
            // Progreso vertical (V) de 0.0f (base) a 1.0f (ápice)
            const float v_del_anillo = static_cast<float>(ring) / static_cast<float>(anillos);

            // El radio disminuye linealmente a medida que subimos hacia el ápice
            const float r_ring = radio * (1.0f - v_del_anillo);
            const float y = v_del_anillo * altura;

            // Bucle diametral (N + 1 vértices para manejar la costura UV)
            for (unsigned i = 0; i <= N; ++i)
            {
                const float u_del_gajo = static_cast<float>(i) / static_cast<float>(N);
                const float th = 2.0f * PI * u_del_gajo;
                const float cos_th = cos(th);
                const float sin_th = sin(th);

                Vertex v;
                // Posición 3D
                v.position = {r_ring * cos_th, y, r_ring * sin_th};

                // Normal inclinada por el semiángulo de conicidad
                v.normal = {cos_th * cos_alpha_2, sin_alpha_2, sin_th * cos_alpha_2};

                // Coordenadas de textura UV
                v.tex_coords = {u_del_gajo, v_del_anillo};

                data.vertices.push_back(v);
            }
        }

        // 2. GENERACIÓN DE ÍNDICES (TIRAS DE TRIÁNGULOS)
        for (unsigned ring = 0; ring < anillos; ++ring)
        {
            const unsigned ring_bottom_offset = ring * (N + 1);
            const unsigned ring_top_offset = (ring + 1) * (N + 1);

            for (unsigned i = 0; i < N; ++i)
            {
                const unsigned b0 = ring_bottom_offset + i;
                const unsigned b1 = ring_bottom_offset + i + 1;
                const unsigned t0 = ring_top_offset + i;
                const unsigned t1 = ring_top_offset + i + 1;

                // Orden antihorario (CCW) visto desde afuera para que la normal apunte hacia afuera
                data.indices.push_back(b0);
                data.indices.push_back(t1);
                data.indices.push_back(b1);

                data.indices.push_back(b0);
                data.indices.push_back(t0);
                data.indices.push_back(t1);
            }
        }

        return data;
    }

    MeshData terreno(int N, float step, const Alturas &alturas)
    {
        MeshData data;

        // Reservamos memoria para los vértices y los índices
        data.vertices.reserve((N + 1) * (N + 1));
        data.indices.reserve(N * N * 6);

        auto get_h = [&](int r, int c)
        {
            int r_clamped = std::clamp(r, 0, N);
            int c_clamped = std::clamp(c, 0, N);
            return alturas(r_clamped, c_clamped);
        };

        // Generamos los vértices
        for (int i = 0; i <= N; ++i)
        {
            for (int j = 0; j <= N; ++j)
            {
                Vertex v;
                float y = alturas(i, j);
                v.position = {j * step, y, i * step}; // Posición en el plano XZ

                float df_dx = (get_h(i, j - 1) - get_h(i, j + 1)) / (2.0f * step);
                float df_dz = (get_h(i - 1, j) - get_h(i + 1, j)) / (2.0f * step);
                v.normal = glm::normalize(glm::vec3(df_dx, 1.0f, df_dz));
                
                v.tex_coords = {static_cast<float>(j) / N, static_cast<float>(i) / N}; // Coordenadas de textura
                data.vertices.push_back(v);
            }
        }

        // Generamos los índices para formar triángulos
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                int topLeft = i * (N + 1) + j;
                int topRight = topLeft + 1;
                int bottomLeft = (i + 1) * (N + 1) + j;
                int bottomRight = bottomLeft + 1;

                // Primer triángulo
                data.indices.push_back(topLeft);
                data.indices.push_back(bottomLeft);
                data.indices.push_back(topRight);

                // Segundo triángulo
                data.indices.push_back(topRight);
                data.indices.push_back(bottomLeft);
                data.indices.push_back(bottomRight);
            }
        }

        return data;
    }
} // namespace primitives