#pragma once

#include <memory>
#include <thread>

#include "terrain_base.h"

#ifndef _DEBUG
#include <atomic>
#endif 

template<typename T>
void random_noise(T& vHeights, const unsigned int scale)
{
    signed int h = scale;
    for (unsigned int i = 0; i < scale; ++i) {
        for (unsigned int j = 0; j < scale; ++j) {
            vHeights[i][j] = terrain_base::random_between(-h, h);
        }
    }
}

template<typename T>
void _tensor_noise(T& vHeights, const unsigned int scale, const unsigned int x, const unsigned int y, const unsigned int radius, const signed int value, const bool dec)
{
    if (radius >= scale)
        return;
    signed int h = scale;
    if (dec) {
        if (value < -h)
            return;
    }
    else if (value > static_cast<signed int>(scale))
        return;
    bool full = true;
    for (unsigned int i = 0; i < 2 * radius + 1; ++i) {
        // Gauche :
        if (radius <= x + i && radius <= y) {
            auto X = x + i - radius;
            auto Y = y - radius;
            if (X < scale && vHeights[X][Y] < value) {
                full = false;
                vHeights[X][Y] = value;
            }
        }
        // Haut :
        if (radius <= y + i && radius <= x) {
            auto Y = y + i - radius;
            auto X = x - radius;
            if (Y < scale && vHeights[X][Y] < value) {
                vHeights[X][Y] = value;
                full = false;
            }
        }
        // Bas :
        if (i <= y + radius && scale > x + radius) {
            auto Y = y + radius - i;
            auto X = x + radius;
            if (Y < scale && vHeights[X][Y] < value) {
                full = false;
                vHeights[X][Y] = value;
            }
        }
        // Droite :
        if (i <= x + radius && scale > y + radius) {
            auto X = x + radius - i;
            auto Y = y + radius;
            if (X < scale && vHeights[X][Y] < value) {
                full = false;
                vHeights[X][Y] = value;
            }
        }
    }
    if (!full) _tensor_noise(vHeights, scale, x, y, radius + 1, dec ? value - 1 : value + 1, dec);
}

template<typename T>
void tensor_noise(T& vHeights, const unsigned int scale)
{
    signed int h = scale;
    for (unsigned int i = 0; i < scale; ++i) {
        for (unsigned int j = 0; j < scale; ++j) {
            vHeights[i][j] = -h;
        }
    }
    for (unsigned int i = 0; i < 1; ++i) {
        auto x = terrain_base::random_between((unsigned int)0, scale - 1);
        auto y = terrain_base::random_between((unsigned int)0, scale - 1);
        signed int h = scale;
        auto temp = vHeights[x][y];
        vHeights[x][y] = terrain_base::random_between(-h, h);
        //bool dec = terrain_base::random_between(0, 1);
        //_tensor_noise(vHeights, scale, x, y, 1, dec ? vHeights[x][y] - 1 : vHeights[x][y] + 1, dec);
        _tensor_noise(vHeights, scale, x, y, 1, vHeights[x][y] - 1, temp > vHeights[x][y]);
    }
    /*for (int i = 0; i < scale; ++i) {
        vHeights[i][0];
        vHeights[0][i];
        vHeights[i][scale-1];
        vHeights[scale-1][i];
    }*/
}

// Sinus cardinal
// https://fr.wikipedia.org/wiki/Sinus_cardinal
template <typename T>
T sinc(T in) {
    // Par convention (Prolongement par continuité) : sinc(0) = 1.
    return in == 0 ? 1 : sin(in) / in;
}

template<typename T>
void volcano(T& vHeights, const unsigned int scale)
{
    // https://knowledgemix.wordpress.com/tag/sinc-function/
    auto matrix = std::make_unique <std::unique_ptr<double[]>[]>(scale);
    for (int i = 0; i < scale; ++i) {
        matrix.get()[i] = std::make_unique<double[]>(scale);
    }
    for (int i = 0; i < scale; ++i) {
        for (int j = 0; j < scale; ++j) {
            //vHeights[i][j] = 255. *  sin(static_cast<double>(j) / 12.) * sin(static_cast<double>(i) / 12.);
            double x, y;
            x = static_cast<double>(i) / 12.;
            y = static_cast<double>(j) / 12.;
            double h = sinc(sin(x)) * sinc(cos(y));
            matrix[i][j] = h; // 512. * (h - .5);
        }
    }
    double min = sin(1.) * sin(1.), max = sinc(0.); // sinc est décroissante sur [0; 1] et symétrique sur [-1; 1].
    // min <= matrix[i][j] <= max
    // 0 <= matrix[i][j] - min <= max - min
    // 0 <= ( matrix[i][j] - min ) / ( max - min ) <= 1 , max != min
    // Normalisation :
    for (int i = 0; i < scale; ++i) {
        for (int j = 0; j < scale; ++j) {
            double h = 512 * (matrix[i][j] - min) / (max - min) - 255;
            vHeights[i][j] = h;
        }
    }
    return;
    signed int h = scale;
    // Au plancher :
    for (unsigned int i = 0; i < scale; ++i) {
        for (unsigned int j = 0; j < scale; ++j) {
            vHeights[i][j] = -1;
        }
    }
    // Volcans :
    for (unsigned int iterations = 0; iterations < 1; ++iterations) {
        auto x = terrain_base::random_between((unsigned int)0, scale - 1);
        auto y = terrain_base::random_between((unsigned int)0, scale - 1);
        auto rayon = terrain_base::random_between((unsigned int)0, (scale - 1) / 2);
        //int k = rayon;
        for (int k = rayon; k > 0; --k) {
            int h = 10 * sqrt(rayon * rayon - k * k);
            //int h = (rayon - k) * (rayon - k) / 10;// sqrt(rayon * rayon - k * k);
            if (h > scale) h = scale;
            for (int i = -k; i <= k; ++i) {
                int j = sqrt(k * k - i * i);
                int ip = (x + i) % scale;
                for (int l = 0; l <= j; ++l) {
                    vHeights[ip][(y + l) % scale] = h;
                    vHeights[ip][(y - l) % scale] = h;
                }
            }
        }
    }
}

/*template<typename T, typename U>
void thrSinusCard( T & matrix, U & _sinc, const unsigned int scale)*/
#ifdef _DEBUG
void thrSinusCard(std::unique_ptr<std::unique_ptr<double[]>[]>& matrix, std::unique_ptr<std::unique_ptr<double[]>[]>& _sinc, const unsigned int scale);
#else
void thrSinusCard(std::unique_ptr<std::unique_ptr<std::atomic<double>[]>[]>& matrix, std::unique_ptr<std::unique_ptr<double[]>[]>& _sinc, const unsigned int scale);
#endif 

template<typename T>
void sinus_cardinal(T& vHeights, const unsigned int scale)
{
    /*auto arrHeigth = std::make_unique <std::unique_ptr<double[]>[]>(scale);
    for (int i = 0; i < scale; ++i) {
        arrHeigth.get()[i] = std::make_unique<double[]>(scale);
    }
    stamp_noise(vHeights, scale);
    center_map(arrHeigth, vHeights, scale, scale);/**/
#ifdef _DEBUG
    auto matrix = std::make_unique <std::unique_ptr<double[]>[]>(scale);
#else
    auto matrix = std::make_unique <std::unique_ptr<std::atomic<double>[]>[]>(scale);
#endif
    for (int i = 0; i < scale; ++i) {
#ifdef _DEBUG
        matrix.get()[i] = std::make_unique<double[]>(scale);
#else
        matrix.get()[i] = std::make_unique<std::atomic<double>[]>(scale);
#endif
    }
    auto _sinc = std::make_unique <std::unique_ptr<double[]>[]>(scale);
    for (int i = 0; i < scale; ++i) {
        _sinc.get()[i] = std::make_unique<double[]>(scale);
    }
    for (int i = 0; i < scale; ++i) {
        for (int j = 0; j < scale; ++j) {
            _sinc[i][j] = sinc(sqrt(i * i + j * j));
        }
    }

    std::vector<std::thread> vThreads;
    for (int thr = 0; thr < 8; ++thr)
        vThreads.emplace_back(std::thread(thrSinusCard, std::ref(matrix), std::ref(_sinc), scale));

    while (!vThreads.empty()) {
        vThreads.back().join();
        vThreads.pop_back();
    }

    // A partir d'ici, la heightmap est terminée. Il n'y a plus qu'à  déterminer les extremums
    // pour normaliser la hauteur.
    long double max = 0, min = 65536;
    for (unsigned int x = 0; x < scale; ++x) {
        for (unsigned int y = 0; y < scale; ++y) {
            if (matrix[x][y] > max) {
                max = matrix[x][y];
            }
            if (matrix[x][y] < min) {
                min = matrix[x][y];
            }
        }
    }
    // min <= matrix[i][j] <= max
    // 0 <= matrix[i][j] - min <= max - min
    // 0 <= ( matrix[i][j] - min ) / ( max - min ) <= 1 , max != min
    // Normalisation :
    for (int i = 0; i < scale; ++i) {
        for (int j = 0; j < scale; ++j) {
            double h = 512 * (matrix[i][j] - min) / (max - min) - 255;
            vHeights[i][j] += h;
            //vHeights[i][j] /= 2;
        }
    }
}
