#pragma once

#include <map>
#include <string>
#include <vector>
#include <Mesh/Mesh.h>

// Lee un .obj (v, vt, vn, f) y lo devuelve listo para Mesh::load().
// Caras de N lados se triangulan en abanico. Si falla, devuelve un MeshData vacio.
// ignorar: nombres de objetos ("o <nombre>") cuyas caras no se cargan.
std::map<std::string, MeshData> load_obj(const std::string &path, const std::vector<std::string> &ignorar = {});
