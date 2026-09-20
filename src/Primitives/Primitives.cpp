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

        // Reservamos memoria para evitar reasignaciones
        data.vertices.reserve(24); // reservar espacio de memoria, tiene la funcion vector
        data.indices.reserve(36);

        // Definición de vértices (4 por cara para permitir un color distinto por cara)
        // El orden de las esquinas en cada cara es ANTIHORARIO mirando desde afuera.
        data.vertices = {
            // CARA FRONTAL (+Z) - Color: Rojo
            {-hx, -hy, hz, 1.0f, 0.0f, 0.0f}, // Inferior Izquierda (0)
            {hx, -hy, hz, 1.0f, 0.0f, 0.0f},  // Inferior Derecha   (1)
            {hx, hy, hz, 1.0f, 0.0f, 0.0f},   // Superior Derecha   (2)
            {-hx, hy, hz, 1.0f, 0.0f, 0.0f},  // Superior Izquierda (3)

            // CARA TRASERA (-Z) - Color: Verde
            {hx, -hy, -hz, 0.0f, 1.0f, 0.0f},  // Inferior Izquierda (4)
            {-hx, -hy, -hz, 0.0f, 1.0f, 0.0f}, // Inferior Derecha   (5)
            {-hx, hy, -hz, 0.0f, 1.0f, 0.0f},  // Superior Derecha   (6)
            {hx, hy, -hz, 0.0f, 1.0f, 0.0f},   // Superior Izquierda (7)

            // CARA IZQUIERDA (-X) - Color: Azul
            {-hx, -hy, -hz, 0.0f, 0.0f, 1.0f}, // Inferior Izquierda (8)
            {-hx, -hy, hz, 0.0f, 0.0f, 1.0f},  // Inferior Derecha   (9)
            {-hx, hy, hz, 0.0f, 0.0f, 1.0f},   // Superior Derecha   (10)
            {-hx, hy, -hz, 0.0f, 0.0f, 1.0f},  // Superior Izquierda (11)

            // CARA DERECHA (+X) - Color: Amarillo
            {hx, -hy, hz, 1.0f, 1.0f, 0.0f},  // Inferior Izquierda (12)
            {hx, -hy, -hz, 1.0f, 1.0f, 0.0f}, // Inferior Derecha   (13)
            {hx, hy, -hz, 1.0f, 1.0f, 0.0f},  // Superior Derecha   (14)
            {hx, hy, hz, 1.0f, 1.0f, 0.0f},   // Superior Izquierda (15)

            // CARA SUPERIOR (+Y) - Color: Magenta
            {-hx, hy, hz, 1.0f, 0.0f, 1.0f},  // Inferior Izquierda (16)
            {hx, hy, hz, 1.0f, 0.0f, 1.0f},   // Inferior Derecha   (17)
            {hx, hy, -hz, 1.0f, 0.0f, 1.0f},  // Superior Derecha   (18)
            {-hx, hy, -hz, 1.0f, 0.0f, 1.0f}, // Superior Izquierda (19)

            // CARA INFERIOR (-Y) - Color: Cían
            {-hx, -hy, -hz, 0.0f, 1.0f, 1.0f}, // Inferior Izquierda (20)
            {hx, -hy, -hz, 0.0f, 1.0f, 1.0f},  // Inferior Derecha   (21)
            {hx, -hy, hz, 0.0f, 1.0f, 1.0f},   // Superior Derecha   (22)
            {-hx, -hy, hz, 0.0f, 1.0f, 1.0f}   // Superior Izquierda (23)
        };

        // Índices de los dos triángulos por cara (patrón 0,1,2 - 2,3,0 para cada conjunto de 4 vértices)
        data.indices = {
            // Cara Frontal
            0, 1, 2, 2, 3, 0,
            // Cara Trasera
            4, 5, 6, 6, 7, 4,
            // Cara Izquierda
            8, 9, 10, 10, 11, 8,
            // Cara Derecha
            12, 13, 14, 14, 15, 12,
            // Cara Superior
            16, 17, 18, 18, 19, 16,
            // Cara Inferior
            20, 21, 22, 22, 23, 20};

        return data;
    }

    /*MeshData cilinder()
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
    }*/
} // namespace primitives