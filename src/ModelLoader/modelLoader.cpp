#include "modelLoader.h"

#include <algorithm>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using glm::vec2;
using glm::vec3;
using std::string;
using std::vector;

// Copia un aiMesh (ya triangulado y con los "loc" aplicados) a un MeshData.
static MeshData convertir_malla(const aiMesh *mesh, const aiMatrix4x4 &transform)
{
    aiMatrix3x3 transform_normal(transform);
    transform_normal.Inverse().Transpose();
    MeshData data;
    data.vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex v{};
        const aiVector3D p = transform * mesh->mVertices[i];
        v.position = vec3(p.x, p.y, p.z);
        if (mesh->HasNormals())
        {
            const aiVector3D n = (transform_normal * mesh->mNormals[i]).Normalize();
            v.normal = vec3(n.x, n.y, n.z);
        }
        if (mesh->HasTextureCoords(0))
        {
            const aiVector3D &t = mesh->mTextureCoords[0][i]; // assimp guarda los UV en un vec3: z no se usa
            v.tex_coords = vec2(t.x, t.y);
        }
        data.vertices.push_back(v);
    }

    data.indices.reserve(mesh->mNumFaces * 3);
    for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
    {
        const aiFace &cara = mesh->mFaces[f];
        if (cara.mNumIndices != 3)
            continue; // puntos o lineas sueltas del .ac: no son triangulos
        for (unsigned int k = 0; k < 3; ++k)
            data.indices.push_back(cara.mIndices[k]);
    }
    return data;
}

// Lee kd, ks, ns y el nombre de la textura difusa de un aiMaterial.
static void convertir_material(const aiMaterial *mat, ModelPart &parte)
{
    aiColor3D kd(1.0f), ks(0.0f);
    float ns = 1.0f;
    mat->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
    mat->Get(AI_MATKEY_COLOR_SPECULAR, ks);
    mat->Get(AI_MATKEY_SHININESS, ns);
    parte.material = Material{vec3(kd.r, kd.g, kd.b), vec3(ks.r, ks.g, ks.b), std::max(ns, 1.0f)}; // ns 0 rompe el pow del especular

    aiString ruta;
    if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &ruta) == AI_SUCCESS)
        parte.textura = ruta.C_Str(); // relativa a la carpeta del modelo, ej. "f16.png"
}

// Recorre el arbol de nodos acumulando transformaciones (padre * hijo) y saltea los nombres de "ignorar".
static void procesar_nodo(const aiScene *scene, const aiNode *nodo, const aiMatrix4x4 &transform_padre,
                          const vector<string> &ignorar, vector<ModelPart> &partes)
{
    if (std::find(ignorar.begin(), ignorar.end(), nodo->mName.C_Str()) != ignorar.end())
        return; // ni el nodo ni sus hijos

    const aiMatrix4x4 transform = transform_padre * nodo->mTransformation;
    for (unsigned int i = 0; i < nodo->mNumMeshes; ++i)
    {
        const aiMesh *mesh = scene->mMeshes[nodo->mMeshes[i]];
        ModelPart parte;
        parte.data = convertir_malla(mesh, transform);
        convertir_material(scene->mMaterials[mesh->mMaterialIndex], parte);
        partes.push_back(std::move(parte));
    }
    for (unsigned int i = 0; i < nodo->mNumChildren; ++i)
        procesar_nodo(scene, nodo->mChildren[i], transform, ignorar, partes);
}

vector<ModelPart> load_model(const string &path, const vector<string> &ignorar)
{
    vector<ModelPart> partes;

    // El Importer es duenio del aiScene: al salir de la funcion se libera todo,
    // por eso copiamos a MeshData antes de devolver.
    Assimp::Importer importer;
    importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 30.0f); // el "crease 30" del .ac
    const aiScene *scene = importer.ReadFile(path,
                                             aiProcess_Triangulate |              // todo a triangulos
                                                 aiProcess_GenSmoothNormals |     // el .ac no trae normales
                                                 aiProcess_JoinIdenticalVertices);
    if (scene == nullptr || scene->mRootNode == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
    {
        std::cerr << "[ModelLoader ERROR] " << path << ": " << importer.GetErrorString() << std::endl;
        return partes;
    }

    procesar_nodo(scene, scene->mRootNode, aiMatrix4x4(), ignorar, partes);

    size_t triangulos = 0;
    for (const ModelPart &parte : partes)
        triangulos += parte.data.indices.size() / 3;

    std::cout << "[ModelLoader] " << path << ": " << partes.size() << " partes, "
              << triangulos << " triangulos" << std::endl;
    return partes;
}