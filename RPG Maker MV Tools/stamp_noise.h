#pragma once

// Le générateur de tampon
template <typename T>
void UNIVERSE_STAMP_1(T& matrix, const int scale) {
    long double halfScale = static_cast<double>(scale) / 2.;
    long double radius;

    auto powersOfTwo = std::make_unique<long double[]>(scale);

    // On crée deux tables contenants les valeurs élevées à  la puissance de deux.
    // On calcul ainsi n fois ces valeurs au lieu de nÂ².
    for (int x = 0; x < scale; ++x) {
        powersOfTwo[x] = (static_cast<double>(x) - halfScale) * (static_cast<double>(x) - halfScale);
    }

    for (int x = 0, X = 0; x < scale; ++x) {
        for (int y = 0; y < scale; ++y) {
            // On calcule le rayon du cercle sur lequel se trouve le point courant.
            // Opération très TRèS gourmande en temps CPU
            radius = sqrtl(powersOfTwo[y] + powersOfTwo[x]);
            if (radius < halfScale) {
                // y a plus qu'à  dessiner le cône.
                matrix[x][y] = (halfScale - radius) / (halfScale);
            }
            else {
                // Si on est en dehors du cercle, on se casse pas la tête et on affecte un zéro.
                matrix[x][y] = 0;
            }
        }
    }
}

template <typename T>
void UNIVERSE_STAMP_NOISE(T& matrix, const T& stamp, const int scale, const int offsetX, const int offsetY, const int realScale) {

    // La condition d'arrêt de notre bruit récursif.
    // Selon la granularité que l'on désire, on peut augmenter la valeur limite de scale.
    if (scale == 1) {
        return;
    }

    // Demi dimension courante
    // Comme scale est une puissance de deux, plutôt que de diviser, on opère une rotation binaire.
    int halfScale = scale >> 1;
    int x, y;

    // Deux variables très importantes, ce sont elles qui déterminent ou sera appliqué le tampon.
    // C'est le positionnement aléatoire qui fait toute la "beauté" de la heightmap.
    int randX = -halfScale + terrain_base::getRandomDist(terrain_base::generator) % scale;
    int randY = -halfScale + terrain_base::getRandomDist(terrain_base::generator) % scale;

    // à€ chaque octave il faut diminuer l'influence du bruit.
    // On se sert également de cette variable comme pas d'incrémentation des
    // coordonnées du tampon.
    int inc = realScale / scale;

    // Deux variables incrémentales qui servent à  récupérer le pixel locale au tampon, en fonction de l'octave.
    // Elles sont toute les deux incrémentés avec la valeur de inc.
    int stampX = 0, stampY = 0;

    // Détermine le signe du tampon.
    // S'il est positif, le terrain se surélève, à  l'inverse, il se creuse
    long double sign = terrain_base::getRandomDist(terrain_base::generator) & 2 ? -1.0 : 1.0;

    int tmpCoordX, tmpCoordY;

    for (x = 0, stampY = 0; x < scale; ++x, stampY = 0, stampX += inc /* On incrémente à  l'échelle la coordonnée locale au tampon */) {
        for (y = 0; y < scale; ++y, stampY += inc /* On incrémente à  l'échelle la coordonnée locale au tampon */) {
            // Avec ce test le gros bloc d'instructions est répété 1.27 fois moins que s'il n'y avait pas de test.
            if (stamp[stampX][stampY] != 0) {
                // On économise des calculs fastidieux en stockant ces valeurs qui seront solicitées plusieurs fois.
                tmpCoordX = (randX + offsetX + x + realScale) % realScale;
                tmpCoordY = (randY + offsetY + y + realScale) % realScale;
                matrix[tmpCoordX][tmpCoordY] += sign * stamp[stampX][stampY] / static_cast<long double>(inc);
            }
        }
    }

    // En divisant par deux la dimension courante à  chaque récursion, et en modifiant l'offset,
    // on subdivise en permanence la heighmap jusqu'à  ce que la dimension ainsi divisée soit égale à  un.
    // En procédant ainsi, on travaille récursivement sur différentes
    // portions de la heighmap. Il y a donc quatre portions par secteur et à  chaque récursion, chacunes
    // des portions devient elles même un secteur.

    // Portion en haut à  gauche du secteur courant
    UNIVERSE_STAMP_NOISE(matrix, stamp, scale / 2, offsetX + 0, offsetY + 0, realScale);

    // Portion en bas à  gauche du secteur courant
    UNIVERSE_STAMP_NOISE(matrix, stamp, scale / 2, offsetX + 0, offsetY + scale / 2, realScale);

    // Portion en bas à  droite du secteur courant
    UNIVERSE_STAMP_NOISE(matrix, stamp, scale / 2, offsetX + scale / 2, offsetY + scale / 2, realScale);

    // Portion en haut à  droite du secteur courant
    UNIVERSE_STAMP_NOISE(matrix, stamp, scale / 2, offsetX + scale / 2, offsetY + 0, realScale);
}

/*
scale : puissance de deux.
*/
template<typename T>
void stamp_noise(T& vHeights, const unsigned int scale)
{
    auto matrix = std::make_unique<std::unique_ptr<long double[]>[]>(scale);
    for (int i = 0; i < scale; ++i)
        matrix[i] = std::make_unique<long double[]>(scale);

    auto stamp = std::make_unique<std::unique_ptr<long double[]>[]>(scale);
    for (int i = 0; i < scale; ++i)
        stamp[i] = std::make_unique<long double[]>(scale);

    // On génère d'abord notre tampon
    UNIVERSE_STAMP_1(stamp, scale);

    // On lance notre bruit récursif.
    // On conmmence la récursion avec l'octave la plus grande.
    UNIVERSE_STAMP_NOISE(matrix, stamp, scale, 0, 0, scale);

    // à€ partir d'ici, la heightmap est terminé. Il n'y a plus qu'à  déterminer les extremums
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

    // 0 <= (matrix[i][j] - min) / (max - min) <= 1
    for (unsigned int j = 0; j < scale; ++j) {
        for (unsigned int i = 0; i < scale; ++i) {
            vHeights[i][j] = 512. * (matrix[i][j] - min) / (max - min) - 255.;
        }
    }
}