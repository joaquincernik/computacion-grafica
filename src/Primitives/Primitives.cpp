#include "Primitives.h"

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

        // Definición de vértices: { Posición (x,y,z), Normal (nx,ny,nz), UV (u,v) }
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

        // Los índices no cambian: conectan los mismos grupos de 4 vértices por cara
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

    MeshData cilinder()
    {
        int v_del_anillo = 0;
        for (unsigned i = 0; i <= N; ++i) // OJO : <= y no <
        {
            const float th = 2.0 f * PI * float(i) / float(N);
            Vertex v;
            v.position = {r * cos(th), y, r * sin(th)};
            v.normal = {cos(th), 0, sin(th)};
            v.tex_coords = {float(i) / float(N), v_del_anillo};
            vertices.push_back(v);
        }

        // indices
        for (unsigned i = 0; i < N; ++i)
        {
            const unsigned b0 = i, b1 = i + 1;                 // anillo de abajo
            const unsigned t0 = N + 1 + i, t1 = N + 1 + i + 1; // anillo de arriba
            indices.insert(indices.end(), {b0, t1, b1});
            indices.insert(indices.end(), {b0, t0, t1});
        }
    }
} // namespace primitives