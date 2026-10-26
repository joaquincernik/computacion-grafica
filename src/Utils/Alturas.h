#pragma once

#include "FastNoiseLite.h"
// Fuente de alturas por ruido procedural. Devuelve la altura YA ESCALADA, en
// las mismas unidades que el resto de la escena.
class Alturas
{
    FastNoiseLite ruido_;
    float amplitud_; // desnivel maximo, en unidades de escena
public:
    Alturas(int semilla, float frecuencia, float amplitud)
        : amplitud_(amplitud)
    {
        ruido_.SetSeed(semilla);
        ruido_.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        ruido_.SetFractalType(FastNoiseLite::FractalType_FBm); // suma de octavas
        ruido_.SetFractalOctaves(5);
        ruido_.SetFrequency(frecuencia);
    }
    // GetNoise devuelve aprox. [-1, 1]: la amplitud lo lleva a escala de la escena.
    float operator()(unsigned i, unsigned j) const
    {
        return amplitud_ * ruido_.GetNoise(float(j), float(i));
    }
};