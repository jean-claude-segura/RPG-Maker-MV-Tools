#pragma once
#include "diamond_square.h"
#include "perlin_noise.h"
#include "stamp_noise.h"
#include "misc_landscape.h"
#include "simplex.h"
#include "opensimplex2.h"
#include "rivers.h"

namespace terrain {

#pragma once
#include <array>
#include <random>
#include <cmath>


    template <typename T>
    std::size_t newSide(const T left, const T right)
    {
        auto maxsize = static_cast<unsigned int>(std::max(left, right));
        unsigned int exp = static_cast<unsigned int>(log2(maxsize));
        auto newsize = exp2(exp2(exp) + 1 >= maxsize ? exp : exp + 1) + 1;
        return static_cast<std::size_t>(newsize);
    }

    template<typename T>
    void CreateLandscape(T& arrHeights, const std::size_t SIZEX, const std::size_t SIZEY, const unsigned int type = 0)
    {
        auto side = type == 0 ? newSide(SIZEY, SIZEX) : (SIZEY > SIZEX ? SIZEY : SIZEX);
        auto vHeights = std::make_unique <std::unique_ptr<int[]>[]>(side);
        for (int i = 0; i < side; ++i) {
            vHeights.get()[i] = std::make_unique<int[]>(side);
        }
        switch (type) {
        default:
        case 0:
            diamond_square(vHeights, static_cast<unsigned int>(side));
            break;
        case 1:
            perlin_noise(vHeights, static_cast<unsigned int>(side));
            break;
        case 2:
            stamp_noise(vHeights, static_cast<unsigned int>(side));
            break;
        case 3:
            random_noise(vHeights, static_cast<unsigned int>(side));
            break;
        case 4:
            tensor_noise(vHeights, static_cast<unsigned int>(side));
            break;
        case 5:
            volcano(vHeights, static_cast<unsigned int>(side));
            break;
        case 6:
            sinus_cardinal(vHeights, static_cast<unsigned int>(side));
            break;
        case 7:
            simplex(vHeights, static_cast<unsigned int>(side));
            break;
        case 8:
            opensimplex2(vHeights, static_cast<unsigned int>(side));
            break;
        }
        if (type == 2) {
            terrain_base::center_map(arrHeights, vHeights, SIZEX, SIZEY);
        }
        else {
            for (int i = 0; i < SIZEX; ++i) {
                for (int j = 0; j < SIZEY; ++j) {
                    arrHeights[i][j] = vHeights[i][j];
                }
                //vHeights.get()[i].reset();
            }
        }
        rivers_dispatch(arrHeights, SIZEX, SIZEY, LANDSCAPE_TYPES::erosion_hydraulic_iter, true);
    }
}
