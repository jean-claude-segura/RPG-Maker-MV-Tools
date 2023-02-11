#pragma once
#include <cstdlib>
#include <array>
#include <vector>
#include <stack>
#include <map>
#include <type_traits>
#include <random>
#include <thread>
#ifndef _DEBUG
#include <atomic>
#endif 

namespace terrain {
    static std::mt19937 generator(1);
    static std::uniform_int_distribution<unsigned long> getRandomDist(0, ULONG_MAX);
    static bool initPerlin = true;

    template <typename T>
    T random_between(const T _min, const T _max)
    {
        T answer = 0;
        if (_max > _min) {
            if constexpr (std::is_same_v<T, double>) {
                std::uniform_real_distribution<T> distribution(_min, _max);
                answer = distribution(generator);
            }
            if constexpr (std::is_same_v<T, int>) {
                std::uniform_int_distribution<T> distribution(_min, _max);
                answer = distribution(generator);
            }
        }
        return answer;
    }

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
	    matrix[0][0] = random_between(-scale, scale);
	    matrix[0][h - 1] = random_between(-scale, scale);
	    matrix[h - 1][h - 1] = random_between(-scale, scale);
	    matrix[h - 1][0] = random_between(-scale, scale);
	    signed int i = h - 1;
	    while (i > 1) {
		    signed int id = i / 2;
            scale /= 2.;
		    // Diamant : le centre de chaque carré prend pour valeur la moyenne des 4 coins
		    // à laquelle on ajoute une valeur aléatoire.
		    for (signed int x = id; x < h; x += i) {  /* début de la phase du diamant */
			    for (signed int y = id; y < h; y += i) {
				    double moyenne = (matrix[x - id][y - id] + matrix[x - id][y + id] + matrix[x + id][y + id] + matrix[x + id][y - id]) / 4;
				    matrix[x][y] = moyenne + random_between(-scale, scale);
			    }
		    }
		    signed int décalage = 0;
		    // Carré : chaque milieu des segments du carré, prend pour valeur la moyenne des 4 points
		    // formant ce diamant ajoutée d'une valeur aléatoire.
		    for (signed int x = 0; x < h; x += id) {  /* début de la phase du carré */
			    if (décalage == 0) {
				    décalage = id;
			    }
			    else {
				    décalage = 0;
			    }
			    for (signed int y = décalage; y < h; y += i) {
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
				    matrix[x][y] = somme / n + random_between(-scale, scale);
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

    int POrg[];
    int P[];
    double PerlinNoise2D(const unsigned int x, const unsigned int y, const unsigned int scale);

    /*
	    vHeights : matrice carrée.
    */
    template<typename T>
    void perlin_noise(T& vHeights, const unsigned int scale)
    {
        if (initPerlin) {
            memcpy(P, POrg, 256 * sizeof(int));
            initPerlin = false;
        }
        else {
            // Il nous faut désordonner la table de permutations :
            for (int i = 0; i < 256; ++i)
                std::swap(P[i], P[random_between(0, 255)]);
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

    template <typename T>
    int center_axe(const T& axeMax, const std::size_t SIZEAXE, const int valeurMax)
    {
        // Recherche des lignes les plus longues.
        std::vector<int> vMaxs;
        for (int k = 0; k < SIZEAXE; ++k) {
            if (axeMax[k] == valeurMax) vMaxs.push_back(k);
        }
        // Calcul des longueurs des plages de lignes :
        std::vector<std::pair<int, int>> vPlages;
        auto itCur = vMaxs.begin();
        auto itPrev = itCur;
        vPlages.emplace_back(std::make_pair(*itCur, 1));
        auto itBase = vPlages.end() - 1;
        int MaxPond = 1;
        for (++itCur; itCur != vMaxs.end(); ++itCur) {
            if (*itPrev == *itCur - 1) {
                ++(itBase->second);
                MaxPond = MaxPond > itBase->second ? MaxPond : itBase->second; // Détermination de la longueur de plage la plus étendue.
                ++itPrev;
            }
            else {
                vPlages.emplace_back(std::make_pair(*itCur, 1));
                itBase = vPlages.end() - 1;
                itPrev = itCur;
            }
        }
        auto itFin = vPlages.end() - 1;
        auto itDebut = vPlages.begin();
        // Test de l'existence d'une plage circulaire :
        if (itFin->first + itFin->second == SIZEAXE && itDebut->first == 0) {
            // Il s'agit d'une plage circulaire car composée de deux sous-plages qui se rejoignent par le bord.
            itDebut->first = itFin->first; // Affectation du point de départ correct.
            itDebut->second = itDebut->second + itFin->second; // Affectation de la longueur correcte.
            MaxPond = MaxPond > itDebut->second ? MaxPond : itDebut->second; // Détermination de la longueur de plage la plus étendue.
            vPlages.pop_back(); // Suppression de la fraction de plage.
        }

        // Suppression des plages moins étendues que la plus étendue :
        /*for (auto itPond = vPlages.begin(); itPond != vPlages.end();) {
            if (itPond->second < MaxPond)
                itPond = vPlages.erase(itPond);
            else
                ++itPond;
        }

        auto itMax = vPlages.begin();*/
        auto itMax = std::find_if(vPlages.begin(), vPlages.end(), [MaxPond](std::pair<int, int>& ref) { return ref.second == MaxPond; });
        return itMax->first + itMax->second / 2;
    }

    template<typename T, typename U>
    void center_map(T& arrHeights, const U& vHeights, const std::size_t SIZEX, const std::size_t SIZEY)
    {
        int initx = 0, inity = 0;

        // Calcul des longueurs des lignes :
        auto iMax = std::make_unique <int[]>(SIZEX);
        auto jMax = std::make_unique <int[]>(SIZEY);
        int valueImax = 0;
        int valueJmax = 0;
        if (SIZEX == SIZEY) {
            // Rotation dans les deux sens :
            for (int i = 0; i < SIZEX; ++i) {
                for (int j = 0; j < SIZEY; ++j) {
                    iMax[i] += vHeights[i][j] < 0 ? 1 : 0;
                    jMax[j] += vHeights[i][j] < 0 ? 1 : 0;
                    if (valueJmax < jMax[j]) valueJmax = jMax[j];
                }
                if (valueImax < iMax[i]) valueImax = iMax[i];
            }
            initx = center_axe(iMax, SIZEX, valueImax);
            inity = center_axe(jMax, SIZEY, valueJmax);
        }
        else if (SIZEX > SIZEY) {
            // Rotation horizontale :
            for (int i = 0; i < SIZEX; ++i) {
                for (int j = 0; j < SIZEY; ++j) iMax[i] += vHeights[i][j] < 0 ? 1 : 0;
                if (valueImax < iMax[i]) valueImax = iMax[i];
            }
            initx = center_axe(iMax, SIZEX, valueImax);
        }
        else {
            // Rotation verticale :
            for (int j = 0; j < SIZEY; ++j) {
                for (int i = 0; i < SIZEX; ++i) jMax[j] += vHeights[i][j] < 0 ? 1 : 0;
                if (valueJmax < jMax[j]) valueJmax = jMax[j];
            }
            inity = center_axe(jMax, SIZEY, valueJmax);
        }
        if (initx != 0 || inity != 0) {
            for (int i = 0; i < SIZEX; ++i) {
                for (int j = 0; j < SIZEY; ++j)
                    arrHeights[i][j] = vHeights[static_cast<std::size_t>(i + initx) % SIZEX][static_cast<std::size_t>(j + inity) % SIZEY];
            }
        }
        else {
            for (int i = 0; i < SIZEX; ++i) {
                for (int j = 0; j < SIZEY; ++j)
                    arrHeights[i][j] = vHeights[i][j];
            }
        }
    }

    // Le générateur de tampon
    template <typename T>
    void UNIVERSE_STAMP_1(T & matrix, const int scale) {
        long double halfScale = static_cast<double>(scale) / 2.;
        long double radius;

        auto powersOfTwo = std::make_unique<long double[]>(scale);

        // On crée deux tables contenants les valeurs élevées à la puissance de deux.
        // On calcul ainsi n fois ces valeurs au lieu de n².
        for (int x = 0; x < scale; ++x) {
            powersOfTwo[x] = (static_cast<double>(x) - halfScale) * (static_cast<double>(x) - halfScale);
        }

        for (int x = 0, X = 0; x < scale; ++x) {
            for (int y = 0; y < scale; ++y) {
                // On calcule le rayon du cercle sur lequel se trouve le point courant.
                // Opération très TRÈS gourmante en temps CPU
                radius = sqrtl(powersOfTwo[y] + powersOfTwo[x]);
                if (radius < halfScale) {
                    // y a plus qu'à dessiner le cône.
                    matrix[x][y] = (halfScale - radius) / (halfScale);
                }
                else {
                    // Si on est en dehors du cercle, on se casse pas la tête et on affecte un zero.
                    matrix[x][y] = 0;
                }
            }
        }
    }

    template <typename T>
    void UNIVERSE_STAMP_NOISE(T & matrix, const T & stamp, const int scale, const int offsetX, const int offsetY, const int realScale) {

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
        int randX = -halfScale + getRandomDist(generator) % scale;
        int randY = -halfScale + getRandomDist(generator) % scale;

        // À chaque octave il faut diminuer l'influence du bruit.
        // On se sert également de cette variable comme pas d'incrémentation des
        // coordonnées du tampon.
        int inc = realScale / scale;

        // Deux variables incrémentales qui servent à récupérer le pixel locale au tampon, en fonction de l'octave.
        // Elles sont toute les deux incrémentés avec la valeur de inc.
        int stampX = 0, stampY = 0;

        // Détermine le signe du tampon.
        // S'il est positif, le terrain se surélève, à l'inverse, il se creuse
        long double sign = getRandomDist(generator) & 2 ? -1.0 : 1.0;

        int tmpCoordX, tmpCoordY;

        for (x = 0, stampY = 0; x < scale; ++x, stampY = 0, stampX += inc /* On incrémente à l'échelle la coordonnée locale au tampon */ ) {
            for (y = 0; y < scale; ++y, stampY += inc /* On incrémente à l'échelle la coordonnée locale au tampon */ ) {
                // Avec ce test le gros bloc d'instructions est répété 1.27 fois moins que s'il n'y avait pas de test.
                if (stamp[stampX][stampY] != 0) {
                    // On économise des calculs fastidieux en stockant ces valeurs qui seront solicitées plusieurs fois.
                    tmpCoordX = (randX + offsetX + x + realScale) % realScale;
                    tmpCoordY = (randY + offsetY + y + realScale) % realScale;
                    matrix[tmpCoordX][tmpCoordY] += sign * stamp[stampX][stampY] / static_cast<long double>(inc);
                }
            }
        }

        // En divisant par deux la dimension courante à chaque récursion, et en modifiant l'offset,
        // on subdivise en permanence la heighmap jusqu'à ce que la dimension ainsi divisée soit égale à un.
        // En procédant ainsi, on travaille récursivement sur différentes
        // portions de la heighmap. Il y a donc quatre portions par secteur et à chaque récursion, chacunes
        // des portions devient elles même un secteur.

        // Portion en haut à gauche du secteur courant
        UNIVERSE_STAMP_NOISE(matrix, stamp, scale / 2, offsetX + 0, offsetY + 0, realScale);

        // Portion en bas à gauche du secteur courant
        UNIVERSE_STAMP_NOISE(matrix, stamp, scale / 2, offsetX + 0, offsetY + scale / 2, realScale);

        // Portion en bas à droite du secteur courant
        UNIVERSE_STAMP_NOISE(matrix, stamp, scale / 2, offsetX + scale / 2, offsetY + scale / 2, realScale);

        // Portion en haut à droite du secteur courant
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

        // À partir d'ici, la heightmap est terminé. Il n'y a plus qu'à déterminer les extremums
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

    template<typename T>
    void random_noise(T& vHeights, const unsigned int scale)
    {
        signed int h = scale;
        for (unsigned int i = 0; i < scale; ++i) {
            for (unsigned int j = 0; j < scale; ++j) {
                vHeights[i][j] = random_between(-h, h);
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
        if(!full) _tensor_noise(vHeights, scale, x, y, radius + 1, dec ? value - 1: value + 1, dec);
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
            auto x = random_between((unsigned int)0, scale - 1);
            auto y = random_between((unsigned int)0, scale - 1);
            signed int h = scale;
            auto temp = vHeights[x][y];
            vHeights[x][y] = random_between(-h, h);
            //bool dec = random_between(0, 1);
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
        return in == 0 ?  1 : sin(in) / in;
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
            auto x = random_between((unsigned int)0, scale - 1);
            auto y = random_between((unsigned int)0, scale - 1);
            auto rayon = random_between((unsigned int)0, (scale - 1) / 2);
            //int k = rayon;
            for (int k = rayon; k > 0; --k) {
                int h = 10 * sqrt(rayon * rayon - k * k);
                //int h = (rayon - k) * (rayon - k) / 10;// sqrt(rayon * rayon - k * k);
                if (h > scale) h = scale;
                for (int i = - k; i <= k; ++i) {
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
        for(int thr = 0; thr < 8; ++thr)
            vThreads.emplace_back(std::thread (terrain::thrSinusCard, std::ref(matrix), std::ref(_sinc), scale));
        
        while (!vThreads.empty()) {
            vThreads.back().join();
            vThreads.pop_back();
        }

        // À partir d'ici, la heightmap est terminée. Il n'y a plus qu'à déterminer les extremums
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

    static std::map<int, int> mPondere;

    template<typename T>
    void _rivers(T& arrHeights, const std::size_t SIZEX, const std::size_t SIZEY, const int x, const int y, std::vector<std::pair<int, int>>& vPath, const bool exhaustive = true)
    {
        std::vector<std::pair<int, int>> vMax;
        int xMin, yMin;
        int cptr = 0;
        for (int i = -1; i < 2; ++i) {
            for (int j = -1; j < 2; ++j) {
                if (i == 0 && j == 0)
                    continue; // On ne repart pas de la même case.
                if (x + i >= 0 && y + j >= 0 && x + i < static_cast<int>(SIZEY) && y + j < static_cast<int>(SIZEY)) {
                    // On s'arrête sur les cases d'eau.
                    if (arrHeights[x + i][y + j] > 0) {
                        if (exhaustive) {
                            auto occ = std::find(vPath.begin(), vPath.end(), std::make_pair<int, int>(x + i, y + j));
                            if (occ != vPath.end())
                                continue;
                        }
					    // On ne prend que les cases plus basses ou de même niveau que la case de départ.
					    // On recherche la ou les plus basses.
					    if (arrHeights[x + i][y + j] <= arrHeights[x][y]) {
						    // La première est toujours la plus basse par défaut.
						    if (vMax.empty()) {
							    vMax.push_back(std::make_pair<int, int>(x + i, y + j));
							    xMin = x + i;
							    yMin = y + j;
						    }
						    // Une case plus basse que les autres élimine toutes les autres.
						    else if (arrHeights[x + i][y + j] < arrHeights[xMin][yMin]) {
							    vMax.clear();
							    vMax.push_back(std::make_pair<int, int>(x + i, y + j));
							    xMin = x + i;
							    yMin = y + j;
						    }
						    // A défaut de case plus basse, on garde celles de même niveau.
						    else if (arrHeights[x + i][y + j] == arrHeights[xMin][yMin]) {
							    vMax.push_back(std::make_pair<int, int>(x + i, y + j));
							    xMin = x + i;
							    yMin = y + j;
						    }
					    }
				    }
                }
            }
        }
        if (vMax.empty()) {
            //_obstacle(arrHeights, SIZEX, SIZEY, x, y, vPath);
        }
        else {
            for (const auto& ref : vMax) {
                // On ne passe pas deux fois sur la même case.
                auto occ = std::find(vPath.begin(), vPath.end(), ref);
                if (occ == vPath.end()) {
                    vPath.push_back(ref);
                    _rivers(arrHeights, SIZEX, SIZEY, ref.first, ref.second, vPath);
                }
                auto index = ref.first + ref.second * static_cast<int>(SIZEY);
                auto weight = mPondere.find(index);
                if (weight == mPondere.end())
                    mPondere[index] = 1;
                else
                    ++weight->second;
            }
        }
    }

    template<typename T>
    bool _levelmin(T& arrHeights, const std::size_t SIZEX, const std::size_t SIZEY, const int x, const int y, std::vector<std::pair<int, int>>& vPath, std::tuple<int, int, int> & candidat)
    {
        bool premier = true;
        for (int i = -1; i < 2; ++i) {
            if (x + i < 0) continue;
            if (x + i > SIZEX - 1) continue;
            for (int j = -1; j < 2; ++j) {
                if (y + j < 0) continue;
                if (y + j > SIZEY - 1) continue;
                // On exclut la case de référence :
                if (i == 0 && j == 0) continue;
                // On ne passe jamais par la même case :
                const std::pair<int, int> ref = std::pair<int, int>(x + i, y + j);
                if (vPath.end() != std::find(vPath.begin(), vPath.end(), ref)) continue;
                // Le premier est toujours valide :
                if (premier)
                {
                    candidat = std::tuple<int, int, int>(x + i, y + j, arrHeights[x + i][y + j]);
                    premier = false;
                }
                else
                {
                    if (arrHeights[x + i][y + j] < std::get<2>(candidat))
                        candidat = std::tuple<int, int, int>(x + i, y + j, arrHeights[x + i][y + j]);
                }
            }
        }
        return !premier;
    }

    template<typename T>
    void _erosion(T& arrHeights, const std::size_t SIZEX, const std::size_t SIZEY, const int x, const int y, std::vector<std::pair<int, int>>& vPath)
    {
        std::tuple<int, int, int> firstcandidat;
        if(!_levelmin(arrHeights, SIZEX, SIZEY, x, y, vPath, firstcandidat))
            return;

        std::vector<std::tuple<int, int, int>> candidats;
        std::vector<std::tuple<int, int, int>> rebuts;
        candidats.emplace_back(firstcandidat);

        for (int i = -1; i < 2; ++i) {
            if (x + i < 0)
                continue;
            if (x + i > SIZEX - 1)
                continue;
            for (int j = -1; j < 2; ++j) {
                if (y + j < 0)
                    continue;
                if (y + j > SIZEY - 1)
                    continue;
                // On exclut la case de référence :
                if (i == 0 && j == 0)
                    continue;
                // On exclut le premier :
                if (x + i == std::get<0>(firstcandidat) && y + j == std::get<1>(firstcandidat))
                    continue;
                // On ne passe jamais par la même case :
                const std::pair<int, int> ref = std::pair<int, int>(x + i, y + j);
                if(vPath.end() != std::find(vPath.begin(), vPath.end(), ref))
                    continue;
                if (arrHeights[x + i][y + j] <= std::get<2>(firstcandidat))
                    candidats.emplace_back(std::tuple<int, int, int>(x + i, y + j, arrHeights[x + i][y + j]));
                else
                    rebuts.emplace_back(std::tuple<int, int, int>(x + i, y + j, arrHeights[x + i][y + j]));
            }
        }
		
        for (const auto& candidat : candidats)
        {
            vPath.push_back(std::pair<int, int>(std::get<0>(candidat), std::get<1>(candidat)));
            //arrHeights[std::get<0>(candidat)][std::get<1>(candidat)] = min(arrHeights[std::get<0>(candidat)][std::get<1>(candidat)], arrHeights[x][y] - 1);
            if (arrHeights[std::get<0>(candidat)][std::get<1>(candidat)] <= 0)
                return;
            _erosion(arrHeights, SIZEX, SIZEY, std::get<0>(candidat), std::get<1>(candidat), vPath);
        }
    }

    template<typename T>
    void _erosioniter(T& arrHeights, const std::size_t SIZEX, const std::size_t SIZEY, const int x, const int y, std::vector<std::pair<int, int>>& vPath)
    {
        std::stack<std::vector<std::tuple<int, int, int>>> cstack;
        do
        {
            cstack.push(std::vector<std::tuple<int, int, int>>);
            {
                std::tuple<int, int, int> firstcandidat;
                if (!_levelmin(arrHeights, SIZEX, SIZEY, x, y, vPath, firstcandidat))
                    break;

                std::vector<std::tuple<int, int, int>>& candidats = cstack.top();

                candidats.emplace_back(firstcandidat);

                for (int i = -1; i < 2; ++i) {
                    if (x + i < 0)
                        continue;
                    if (x + i > SIZEX - 1)
                        continue;
                    for (int j = -1; j < 2; ++j) {
                        if (y + j < 0)
                            continue;
                        if (y + j > SIZEY - 1)
                            continue;
                        // On exclut la case de référence :
                        if (i == 0 && j == 0)
                            continue;
                        // On exclut le premier :
                        if (x + i == std::get<0>(firstcandidat) && y + j == std::get<1>(firstcandidat))
                            continue;
                        // On ne passe jamais par la même case :
                        const std::pair<int, int> ref = std::pair<int, int>(x + i, y + j);
                        if (vPath.end() != std::find(vPath.begin(), vPath.end(), ref))
                            continue;
                        if (arrHeights[x + i][y + j] <= std::get<2>(firstcandidat))
                            candidats.emplace_back(std::tuple<int, int, int>(x + i, y + j, arrHeights[x + i][y + j]));
                    }

                }
            }
            while (!cstack.top().empty())
            {
                std::tuple<int, int, int> firstcandidat;
                if (!_levelmin(arrHeights, SIZEX, SIZEY, x, y, vPath, firstcandidat))
                    break;

                std::vector<std::tuple<int, int, int>>& candidats = cstack.top();

                candidats.emplace_back(firstcandidat);

                for (int i = -1; i < 2; ++i) {
                    if (x + i < 0)
                        continue;
                    if (x + i > SIZEX - 1)
                        continue;
                    for (int j = -1; j < 2; ++j) {
                        if (y + j < 0)
                            continue;
                        if (y + j > SIZEY - 1)
                            continue;
                        // On exclut la case de référence :
                        if (i == 0 && j == 0)
                            continue;
                        // On exclut le premier :
                        if (x + i == std::get<0>(firstcandidat) && y + j == std::get<1>(firstcandidat))
                            continue;
                        // On ne passe jamais par la même case :
                        const std::pair<int, int> ref = std::pair<int, int>(x + i, y + j);
                        if (vPath.end() != std::find(vPath.begin(), vPath.end(), ref))
                            continue;
                        if (arrHeights[x + i][y + j] <= std::get<2>(firstcandidat))
                            candidats.emplace_back(std::tuple<int, int, int>(x + i, y + j, arrHeights[x + i][y + j]));
                    }

                    for (const auto& candidat : candidats)
                    {
                        vPath.push_back(std::pair<int, int>(std::get<0>(candidat), std::get<1>(candidat)));

                        if (arrHeights[std::get<0>(candidat)][std::get<1>(candidat)] <= 0)
                            continue;
                        //_erosion(arrHeights, SIZEX, SIZEY, std::get<0>(candidat), std::get<1>(candidat), vPath);
                    }
                }
            };
            cstack.pop();
        } while (!cstack.empty());

        //for (const auto& candidat : candidats)
        //{
        //    vPath.push_back(std::pair<int, int>(std::get<0>(candidat), std::get<1>(candidat)));
        //    //arrHeights[std::get<0>(candidat)][std::get<1>(candidat)] = min(arrHeights[std::get<0>(candidat)][std::get<1>(candidat)], arrHeights[x][y] - 1);
        //    if (arrHeights[std::get<0>(candidat)][std::get<1>(candidat)] <= 0)
        //        break;
        //    _erosion(arrHeights, SIZEX, SIZEY, std::get<0>(candidat), std::get<1>(candidat), vPath);
        //}
    }

    template<typename T>
    void level(T& arrHeights, const std::size_t SIZEX, const std::size_t SIZEY, const int x, const int y)
    {
        for (int i = -1; i < 2; ++i) {
            for (int j = -1; j < 2; ++j) {
                if (x + i >= 0 && y + j >= 0 && x + i < static_cast<int>(SIZEY) && y + j < static_cast<int>(SIZEY)) {
                    --arrHeights[x + i][y + j];
                }
            }
        }
    }

    template<typename T>
    void rivers(T& arrHeights, const std::size_t SIZEX, const std::size_t SIZEY)
    {
        // Recherche de la zone la plus haute.
        int x = 0, y = 0;
        for (int  i = 0; i < SIZEX; ++i) {
            for (int  j = 0; j < SIZEY; ++j) {
                if (arrHeights[i][j] > arrHeights[x][y]) {
                    x = i;
                    y = j;
                }
            }
        }
        mPondere.clear();
        std::vector<std::pair<int, int>> vPath;
        //_rivers(arrHeights, SIZEX, SIZEY, x, y, vPath);
        _erosion(arrHeights, SIZEX, SIZEY, x, y, vPath);
        for (const auto& ref : vPath) {
            arrHeights[ref.first][ref.second] = -1;
        }
        /*if (!mPondere.empty()) {
            for (const auto& ref : mPondere) {
                arrHeights[ref.first % SIZEY][ref.first / SIZEY] = -1;
            }
        }*/
    }

    template<typename T>
    void rivers_arrayed(T & arrHeights, const std::size_t SIZEX, const std::size_t SIZEY)
    {
        // Recherche des extrema locaux.
        std::vector<std::tuple<int, int, int>> vExtrema;
        for (int i = 0; i < SIZEX; ++i) {
            for (int j = 0; j < SIZEY; ++j) {
                vExtrema.push_back(std::tuple<int, int, int>(i, j, arrHeights[i][j]));
            }
        }

        std::sort(vExtrema.begin(), vExtrema.end(),
            [](std::tuple<int, int, int> left, std::tuple<int, int, int> right)
            {
                return  std::get<2>(left) > std::get<2>(right);
            }
        );

        //int limite = 5;
        auto hmax = std::get<2>(vExtrema.front());
        mPondere.clear();
        for (const auto& item : vExtrema)
        {
            auto x = std::get<0>(item);
            auto y = std::get<1>(item);
            std::vector<std::pair<int, int>> vPath;
            _rivers(arrHeights, SIZEX, SIZEY, x, y, vPath);
            for (const auto& ref : vPath) {
                arrHeights[ref.first][ref.second] = -1;
            }
            if (hmax != std::get<2>(item))
                break;
            else
                hmax = std::get<2>(item);
            //if (--limite < 0) break;
        }

        for (std::vector<std::tuple<int, int, int>>::iterator it = vExtrema.begin(); it != vExtrema.end();)
        {
            //delete* it;
            it = vExtrema.erase(it);
        }
    }

    template <typename T>
    std::size_t newSide(const T left, const T right)
    {
        auto maxsize = static_cast<unsigned int>(max(left, right));
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
        case 0 :
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
        }
        if (type == 2) {
            center_map(arrHeights, vHeights, SIZEX, SIZEY);
        }
        else {
            for (int i = 0; i < SIZEX; ++i) {
                for (int j = 0; j < SIZEY; ++j) {
                    arrHeights[i][j] = vHeights[i][j];
                }
                //vHeights.get()[i].reset();
            }
        }
        //rivers_arrayed(arrHeights, SIZEX, SIZEY);
        //rivers(arrHeights, SIZEX, SIZEY);
    }
}
