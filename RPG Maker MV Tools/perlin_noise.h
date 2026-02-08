#pragma once

#include "terrain_base.h"

extern int POrg[256];
extern int P[256];

double PerlinNoise2D(const unsigned int x, const unsigned int y, const unsigned int scale);

/*
    vHeights : matrice carrée.
*/
template<typename T>
void perlin_noise(T& vHeights, const unsigned int scale)
{
    if (terrain_base::initPerlin) {
        memcpy(P, POrg, 256 * sizeof(int));
        terrain_base::initPerlin = false;
    }
    else {
        // Il nous faut désordonner la table de permutations :
        for (int i = 0; i < 256; ++i)
            std::swap(P[i], P[terrain_base::random_between(0, 255)]);
    }

    // Matrice de 256 pixels² simulé avec un tableau de longueur 256²
    // C'est dans ce tableau que nous allons stocker notre heightmap
    auto grid = std::make_unique<std::unique_ptr<double[]>[]>(scale);
    for (int i = 0; i < scale; ++i)
        grid[i] = std::make_unique<double[]>(scale);

    // i et j correspondent respectivement aux axes x et y.
    // k correspond à l'octave courrante.
    unsigned int i, j, k;

    double min = 2., max = 0.;

    // Selon le type de texture on peut ne pas utiliser de coef ou
    // l'utiliser différemment. Mais l'idée ici est de diminuer
    // l'influence du bruit à mesure que la fréquence augmente.
    double coef = 1.0;

    for (j = 0; j < scale; ++j) {
        for (i = 0; i < scale; ++i) {
            coef = 1.0;
            grid[i][j] = 0;
            for (k = scale / 2; k >= 1; k /= 2) {
                grid[i][j] += PerlinNoise2D(i, j, k) * coef;
                coef /= 2.0;
            }
            if (min > grid[i][j]) {
                min = grid[i][j];
            }
            if (max < grid[i][j]) {
                max = grid[i][j];
            }
        }
    }

    // Ici la texture est terminée. Il ne reste plus qu'à la normaliser en
    // vue de l'exploiter.
    // 0 <= (grid[i][j] - min) / (max - min) <= 1
    for (j = 0; j < scale; ++j) {
        for (i = 0; i < scale; ++i) {
            //vHeights[i][j] = (grid[i + j * scale] - min) * (1.0 / (max - min));
            vHeights[i][j] = 512 * (grid[i][j] - min) / (max - min) - 255;
        }
    }
}
