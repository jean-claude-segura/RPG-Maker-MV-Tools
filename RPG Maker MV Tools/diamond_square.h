#pragma once

#include "terrain_base.h"

/*
    vHeights : carré de la forme 2^n + 1.
*/
template<typename T>
void diamond_square(T& vHeights, const unsigned int size)
{
    auto matrix = std::make_unique <std::unique_ptr<double[]>[]>(size);
    for (int i = 0; i < size; ++i) {
        matrix.get()[i] = std::make_unique<double[]>(size);
    }
    signed int h = size;// matrix.size();//t.coté()
    double scale = 1;// matrix.size();//t.coté()
    /* initialisation des coins */
    matrix[0][0] = terrain_base::random_between(-scale, scale);
    matrix[0][h - 1] = terrain_base::random_between(-scale, scale);
    matrix[h - 1][h - 1] = terrain_base::random_between(-scale, scale);
    matrix[h - 1][0] = terrain_base::random_between(-scale, scale);
    signed int i = h - 1;
    while (i > 1) {
        signed int id = i / 2;
        scale /= 2.;
        // Diamant : le centre de chaque carré prend pour valeur la moyenne des 4 coins
        // A  laquelle on ajoute une valeur aléatoire.
        for (signed int x = id; x < h; x += i) {  /* début de la phase du diamant */
            for (signed int y = id; y < h; y += i) {
                double moyenne = (matrix[x - id][y - id] + matrix[x - id][y + id] + matrix[x + id][y + id] + matrix[x + id][y - id]) / 4;
                matrix[x][y] = moyenne + terrain_base::random_between(-scale, scale);
            }
        }
        signed int decalage = 0;
        // Carré : chaque milieu des segments du carré, prend pour valeur la moyenne des 4 points
        // formant ce diamant ajouté d'une valeur aléatoire.
        for (signed int x = 0; x < h; x += id) {  /* début de la phase du carré */
            if (decalage == 0) {
                decalage = id;
            }
            else {
                decalage = 0;
            }
            for (signed int y = decalage; y < h; y += i) {
                double somme = 0;
                double n = 0;
                if (x >= id) {
                    somme = somme + matrix[x - id][y];
                    n = n + 1;
                }
                if (x + id < h) {
                    somme = somme + matrix[x + id][y];
                    n = n + 1;
                }
                if (y >= id) {
                    somme = somme + matrix[x][y - id];
                    n = n + 1;
                }
                if (y + id < h) {
                    somme = somme + matrix[x][y + id];
                    n = n + 1;
                }
                matrix[x][y] = somme / n + terrain_base::random_between(-scale, scale);
            }
        }
        i = id;
    }
    double max = -2, min = 2;
    for (unsigned int x = 0; x < size; ++x) {
        for (unsigned int y = 0; y < size; ++y) {
            if (matrix[x][y] > max) {
                max = matrix[x][y];
            }
            if (matrix[x][y] < min) {
                min = matrix[x][y];
            }
        }
    }
    for (unsigned int j = 0; j < size; ++j) {
        for (unsigned int i = 0; i < size; ++i) {
            vHeights[i][j] = 512 * ((matrix[i][j]) - min) / (max - min) - 255;
        }
    }
}