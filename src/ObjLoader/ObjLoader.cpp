#include "ObjLoader.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>

using glm::vec2;
using glm::vec3;
using std::string;
using std::vector;

// Convierte un indice OBJ (empieza en 1, negativo = relativo al final) a indice 0-based.
// Devuelve -1 si el campo esta vacio o fuera de rango.
static int resolver_indice(const string &campo, size_t cantidad)
{
    if (campo.empty())
        return -1;
    int i = std::stoi(campo);
    i = (i > 0) ? i - 1 : static_cast<int>(cantidad) + i;
    return (i >= 0 && static_cast<size_t>(i) < cantidad) ? i : -1;
}

struct ImageData {
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<unsigned char> pixels;
};

MeshData load_obj(const string &path, const vector<string> &ignorar)
{
    MeshData data;
    std::ifstream archivo(path);
    if (!archivo.is_open())
    {
        std::cerr << "[ObjLoader ERROR] No se pudo abrir: " << path << std::endl;
        return data;
    }

    vector<vec3> posiciones, normales;
    vector<vec2> uvs;
    string linea;
    bool saltear = false; // true mientras estamos dentro de un objeto a ignorar

    while (std::getline(archivo, linea))
    {
        std::istringstream ss(linea);
        string tipo;
        ss >> tipo;

        if (tipo == "o")
        {
            string nombre;
            std::getline(ss >> std::ws, nombre);
            saltear = std::find(ignorar.begin(), ignorar.end(), nombre) != ignorar.end();
        }
        else if (tipo == "v")
        {
            vec3 p;
            ss >> p.x >> p.y >> p.z;
            posiciones.push_back(p);
        }
        else if (tipo == "vn")
        {
            vec3 n;
            ss >> n.x >> n.y >> n.z;
            normales.push_back(n);
        }
        else if (tipo == "vt")
        {
            vec2 t;
            ss >> t.x >> t.y;
            uvs.push_back(t);
        }
        else if (tipo == "f" && !saltear)
        {
            // Cada token es "v", "v/vt", "v//vn" o "v/vt/vn".
            vector<unsigned int> esquinas;
            bool sin_normal = false;
            string token;
            while (ss >> token)
            {
                std::istringstream ts(token);
                string sv, st, sn;
                std::getline(ts, sv, '/');
                std::getline(ts, st, '/');
                std::getline(ts, sn, '/');

                int iv = resolver_indice(sv, posiciones.size());
                if (iv < 0)
                    continue; // vertice invalido: se descarta la esquina
                int it = resolver_indice(st, uvs.size());
                int in = resolver_indice(sn, normales.size());

                Vertex vert{};
                vert.position = posiciones[iv];
                vert.tex_coords = (it >= 0) ? uvs[it] : vec2(0.0f);
                vert.normal = (in >= 0) ? normales[in] : vec3(0.0f);
                sin_normal |= (in < 0);

                data.vertices.push_back(vert);
                esquinas.push_back(static_cast<unsigned int>(data.vertices.size() - 1));
            }
            if (esquinas.size() < 3)
                continue;

            // Sin normales en el archivo: normal plana de la cara.
            if (sin_normal)
            {
                const vec3 &p0 = data.vertices[esquinas[0]].position;
                const vec3 &p1 = data.vertices[esquinas[1]].position;
                const vec3 &p2 = data.vertices[esquinas[2]].position;
                vec3 n = glm::cross(p1 - p0, p2 - p0);
                n = (glm::length(n) > 0.0f) ? glm::normalize(n) : vec3(0.0f, 1.0f, 0.0f);
                for (unsigned int e : esquinas)
                    data.vertices[e].normal = n;
            }

            // Triangulacion en abanico: (c0, c1, c2), (c0, c2, c3), ...
            // ponytail: vertices sin deduplicar (uno por esquina); usar un mapa v/vt/vn -> indice si la memoria importa.
            for (size_t k = 1; k + 1 < esquinas.size(); ++k)
            {
                data.indices.push_back(esquinas[0]);
                data.indices.push_back(esquinas[k]);
                data.indices.push_back(esquinas[k + 1]);
            }
        }
        // Resto (#, g, s, usemtl, mtllib): se ignora.
        // Ojo: v/vn/vt de objetos ignorados SI se guardan, porque los indices de las caras son globales al archivo.
    }

    std::cout << "[ObjLoader] " << path << ": " << data.vertices.size() << " vertices, "
              << data.indices.size() / 3 << " triangulos" << std::endl;
    return data;
}
