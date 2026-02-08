#pragma once

#include "terrain_base.h"

#include <array>

static std::array<int, 512> perm;

inline void buildPermutation(int seed) {
    std::array<int, 256> p;
    for (int i = 0; i < 256; ++i) p[i] = i;

    std::mt19937 rng(seed);
    std::shuffle(p.begin(), p.end(), rng);

    for (int i = 0; i < 512; ++i)
        perm[i] = p[i & 255];
}

inline int fastfloor(double x) {
    int xi = static_cast<int>(x);
    return x < xi ? xi - 1 : xi;
}

inline double dot(int g[2], double x, double y) {
    return g[0] * x + g[1] * y;
}

static int grad2[8][2] = {
    {1,0},{-1,0},{0,1},{0,-1},
    {1,1},{-1,1},{1,-1},{-1,-1}
};

double simplex2d(double x, double y);

double simplex_2d(double x, double y, int seed);

template<typename T>
void simplex(T& vHeights, unsigned int scale)
{
    const int seed = terrain_base::generator();

    const int octaves = 6;
    const double persistence = 0.5;
    const double world_scale = 128;

    double minVal = 1e9;
    double maxVal = -1e9;

    // Tableau temporaire en double
    std::vector<std::vector<double>> grid(scale, std::vector<double>(scale));

    for (unsigned int j = 0; j < scale; ++j) {
        for (unsigned int i = 0; i < scale; ++i) {

            // --- Domain warp (une seule fois) ---
            double wx = simplex_2d(i * 0.02, j * 0.02, seed) * 4.0;
            double wy = simplex_2d(i * 0.02, j * 0.02, seed + 1) * 4.0;

            double x = i + wx;
            double y = j + wy;

            // --- Accumulation des octaves ---
            double value = 0.0;

            for (int o = 0; o < octaves; ++o) {
                double freq = (1 << o) / world_scale;
                double amp = pow(persistence, o);

                value += simplex_2d(
                    x * freq,
                    y * freq,
                    seed
                ) * amp;
            }

            minVal = (value < minVal) ? value : minVal;
            maxVal = (value > maxVal) ? value : maxVal;

            grid[i][j] = value;
        }
    }

    // --- Normalisation ---
    double range = maxVal - minVal;
    if (range < 1e-12) range = 1.0;

    for (unsigned int j = 0; j < scale; ++j) {
        for (unsigned int i = 0; i < scale; ++i) {
            vHeights[i][j] =
                512.0 * (grid[i][j] - minVal) / range - 255.0;
        }
    }
}
