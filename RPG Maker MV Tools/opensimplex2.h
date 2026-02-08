#pragma once

#include "terrain_base.h"

#include <array>
#include <random>

class OpenSimplex2S {
public:
    explicit OpenSimplex2S(int seed);

    double noise2(double x, double y);

private:
    std::array<int, 256> perm;
    std::array<int, 256> permGrad2;

    static const int GRADIENTS_2D_COUNT = 16;
    static const double GRADIENTS_2D[16][2];

    static inline double fastfloor(double x) {
        int xi = (int)x;
        return x < xi ? xi - 1 : xi;
    }
};

template<typename T>
void opensimplex2(T& vHeights, unsigned int scale)
{
    const int seed = terrain_base::generator();

    OpenSimplex2S noise(seed);
    OpenSimplex2S warpX(seed + 12345);
    OpenSimplex2S warpY(seed + 54321);

    const int octaves = 6;
    const double persistence = 0.5;
    const double world_scale = 128.0;

    double minVal = 1e9;
    double maxVal = -1e9;

    // Buffer temporaire en double
    std::vector<std::vector<double>> grid(scale, std::vector<double>(scale));

    for (unsigned int j = 0; j < scale; ++j) {
        for (unsigned int i = 0; i < scale; ++i) {

            // --- Domain warp OpenSimplex2S ---
            double wx = warpX.noise2(i * 0.02, j * 0.02) * 4.0;
            double wy = warpY.noise2(i * 0.02, j * 0.02) * 4.0;

            double x = i + wx;
            double y = j + wy;

            // --- Accumulation des octaves ---
            double value = 0.0;

            for (int o = 0; o < octaves; ++o) {
                double freq = (1 << o) / world_scale;
                double amp = std::pow(persistence, o);

                value += noise.noise2(x * freq, y * freq) * amp;
            }

            minVal = (value < minVal) ? value : minVal;
            maxVal = (value > maxVal) ? value : maxVal;

            grid[i][j] = value;
        }
    }

    // --- Normalisation vers [-255, +255] ---
    double range = maxVal - minVal;
    if (range < 1e-12) range = 1.0;

    for (unsigned int j = 0; j < scale; ++j) {
        for (unsigned int i = 0; i < scale; ++i) {
            vHeights[i][j] =
                static_cast<int>(512.0 * (grid[i][j] - minVal) / range - 255.0);
        }
    }
}
