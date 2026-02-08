#pragma once

#include <random>

namespace terrain_base {
    static std::mt19937 generator(1);
    static std::uniform_int_distribution<unsigned long> getRandomDist(0, ULONG_MAX);
    static bool initPerlin = true;

    template <typename T>
    T random_between(const T _min, const T _max)
    {
        static_assert(std::is_arithmetic_v<T>, "random_between requires arithmetic type");
        T answer = 0;
        if (_max > _min) {
            if constexpr (std::is_same_v<T, double>) {
                std::uniform_real_distribution<T> distribution(_min, _max);
                answer = distribution(generator);
            }
            else if constexpr (std::is_same_v<T, int>) {
                std::uniform_int_distribution<T> distribution(_min, _max);
                answer = distribution(generator);
            }
        }
        return answer;
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
        std::fill_n(iMax.get(), SIZEX, 0);
        std::fill_n(jMax.get(), SIZEY, 0);
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
}