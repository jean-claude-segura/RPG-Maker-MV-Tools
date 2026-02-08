#pragma once

#include <map>
#include <vector>
#include <stack>
#include <limits>

static std::map<int, int> mPondere;

// -----------------------------------------------------------------------------
// _rivers
// Recherche récursive d’un chemin descendant (rivière) à partir d’un point donné.
// Explore les voisins plus bas ou de même niveau, évite les cycles via vPath,
// et accumule un poids de passage dans mPondere.
// -----------------------------------------------------------------------------
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
            if (x + i >= 0 && y + j >= 0 && x + i < static_cast<int>(SIZEX) && y + j < static_cast<int>(SIZEY)) {
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

// -----------------------------------------------------------------------------
// _levelmin
// Recherche tous les voisins valides ayant la hauteur minimale autour d’un point.
// Exclut les cases déjà visitées (vPath). Renvoie false si aucun minimum trouvé.
// -----------------------------------------------------------------------------
template<typename T>
bool _levelmin(
    T& arrHeights,
    std::size_t SIZEX,
    std::size_t SIZEY,
    int x, int y,
    const std::vector<std::pair<int, int>>& vPath,
    std::vector<std::tuple<int, int, int>>& out)
{
    int hmin = std::numeric_limits<int>::max();
    out.clear();

    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {

            if (dx == 0 && dy == 0) continue;

            int nx = x + dx;
            int ny = y + dy;

            if (nx < 0 || ny < 0 || nx >= (int)SIZEX || ny >= (int)SIZEY)
                continue;

            if (std::find(vPath.begin(), vPath.end(), std::make_pair(nx, ny)) != vPath.end())
                continue;

            int h = arrHeights[nx][ny];

            if (h < hmin) {
                hmin = h;
                out.clear();
                out.emplace_back(nx, ny, h);
            }
            else if (h == hmin) {
                out.emplace_back(nx, ny, h);
            }
        }
    }

    return !out.empty();
}

// -----------------------------------------------------------------------------
// _erosion_hydraulic
// Version récursive simplifiée d’érosion hydraulique :
// - l’eau transporte du sédiment,
// - creuse en descendant,
// - dépose en montant,
// - s’évapore progressivement,
// - suit toujours le minimum absolu.
// -----------------------------------------------------------------------------
template<typename T>
void _erosion_hydraulic(
    T& arrHeights,
    std::size_t SIZEX,
    std::size_t SIZEY,
    int x, int y,
    std::vector<std::pair<int, int>>& vPath)
{
    using Node = std::tuple<int, int, int>;

    float water = 1.0f;        // quantité d'eau transportée
    float sediment = 0.0f;     // sédiment transporté
    const float erosionRate = 0.2f;
    const float depositionRate = 0.1f;
    const float evaporation = 0.05f;

    std::stack<std::pair<int, int>> st;
    st.push({ x, y });

    while (!st.empty())
    {
        auto [cx, cy] = st.top();
        st.pop();

        std::vector<Node> candidats;
        if (!_levelmin(arrHeights, SIZEX, SIZEY, cx, cy, vPath, candidats))
            break;

        // Choix du minimum absolu (hydraulique = direction unique)
        auto best = *std::min_element(
            candidats.begin(), candidats.end(),
            [](const Node& a, const Node& b) {
                return std::get<2>(a) < std::get<2>(b);
            }
        );

        int nx = std::get<0>(best);
        int ny = std::get<1>(best);
        int hcur = arrHeights[cx][cy];
        int hnext = arrHeights[nx][ny];

        vPath.emplace_back(nx, ny);

        // Érosion si on descend
        if (hnext < hcur)
        {
            float dh = float(hcur - hnext);
            float erode = erosionRate * dh;

            arrHeights[cx][cy] -= int(erode);
            sediment += erode;
        }
        else
        {
            // Dépôt si on ne descend pas
            float deposit = depositionRate * sediment;
            arrHeights[cx][cy] += int(deposit);
            sediment -= deposit;
        }

        // Évaporation progressive
        water -= evaporation;
        if (water <= 0.1f)
            break;

        // Si on atteint l'eau ou une zone creusée
        if (arrHeights[nx][ny] <= 0)
            break;

        st.push({ nx, ny });
    }
}

// -----------------------------------------------------------------------------
// _erosion_hydraulic_iter
// Version itérative (pile) de l’érosion hydraulique simplifiée.
// Même logique que _erosion_hydraulic mais sans récursion.
// -----------------------------------------------------------------------------
template<typename T>
void _erosion_hydraulic_iter(
    T& arrHeights,
    std::size_t SIZEX,
    std::size_t SIZEY,
    int x, int y,
    std::vector<std::pair<int, int>>& vPath)
{
    using Node = std::tuple<int, int, int>;

    float water = 1.0f;
    float sediment = 0.0f;
    const float erosionRate = 0.2f;
    const float depositionRate = 0.1f;
    const float evaporation = 0.05f;

    std::stack<std::pair<int, int>> st;
    st.push({ x, y });

    while (!st.empty())
    {
        auto [cx, cy] = st.top();
        st.pop();

        std::vector<Node> candidats;
        if (!_levelmin(arrHeights, SIZEX, SIZEY, cx, cy, vPath, candidats))
            break;

        // Choix du minimum absolu
        auto best = *std::min_element(
            candidats.begin(), candidats.end(),
            [](const Node& a, const Node& b) {
                return std::get<2>(a) < std::get<2>(b);
            }
        );

        int nx = std::get<0>(best);
        int ny = std::get<1>(best);

        vPath.emplace_back(nx, ny);

        int hcur = arrHeights[cx][cy];
        int hnext = arrHeights[nx][ny];

        // Érosion si on descend
        if (hnext < hcur)
        {
            float dh = float(hcur - hnext);
            float erode = erosionRate * dh;

            arrHeights[cx][cy] -= int(erode);
            sediment += erode;
        }
        else
        {
            // Dépôt si on ne descend pas
            float deposit = depositionRate * sediment;
            arrHeights[cx][cy] += int(deposit);
            sediment -= deposit;
        }

        // Évaporation
        water -= evaporation;
        if (water <= 0.1f)
            break;

        // Arrêt si on atteint l'eau
        if (arrHeights[nx][ny] <= 0)
            break;

        st.push({ nx, ny });
    }
}

// -----------------------------------------------------------------------------
// _erosion
// Version récursive d’une érosion simple :
// suit tous les voisins ayant la hauteur minimale,
// évite les cycles via vPath,
// s’arrête sur les zones d’eau ou déjà creusées.
// -----------------------------------------------------------------------------
template<typename T>
void _erosion(
    T& arrHeights,
    std::size_t SIZEX,
    std::size_t SIZEY,
    int x, int y,
    std::vector<std::pair<int, int>>& vPath)
{
    using Node = std::tuple<int, int, int>; // x, y, h

    std::vector<Node> candidats;
    if (!_levelmin(arrHeights, SIZEX, SIZEY, x, y, vPath, candidats))
        return;

    for (const auto& c : candidats)
    {
        int nx = std::get<0>(c);
        int ny = std::get<1>(c);

        // Empêche les cycles
        if (std::find(vPath.begin(), vPath.end(), std::make_pair(nx, ny)) != vPath.end())
            continue;

        vPath.emplace_back(nx, ny);

        // Si on atteint l'eau ou une zone déjà creusée, on s'arrête
        if (arrHeights[nx][ny] <= 0)
            continue;

        _erosion(arrHeights, SIZEX, SIZEY, nx, ny, vPath);
    }
}

// -----------------------------------------------------------------------------
// _erosion_iter
// Version itérative (pile) de l’érosion simple.
// Reproduit exactement la logique de _erosion mais sans récursion.
// -----------------------------------------------------------------------------
template<typename T>
void _erosion_iter(
    T& arrHeights,
    std::size_t SIZEX,
    std::size_t SIZEY,
    int x, int y,
    std::vector<std::pair<int, int>>& vPath)
{
    using Node = std::tuple<int, int, int>; // x, y, h

    std::stack<std::pair<int, int>> st;
    st.push({ x, y });

    while (!st.empty())
    {
        auto [cx, cy] = st.top();
        st.pop();

        std::vector<Node> candidats;
        if (!_levelmin(arrHeights, SIZEX, SIZEY, cx, cy, vPath, candidats))
            continue;

        for (const auto& c : candidats)
        {
            int nx = std::get<0>(c);
            int ny = std::get<1>(c);

            // Empêche les cycles
            if (std::find(vPath.begin(), vPath.end(), std::make_pair(nx, ny)) != vPath.end())
                continue;

            vPath.emplace_back(nx, ny);

            // Si on atteint l'eau ou une zone creusée
            if (arrHeights[nx][ny] <= 0)
                continue;

            st.push({ nx, ny });
        }
    }
}

// -----------------------------------------------------------------------------
// level
// Abaisse toutes les cases autour d’un point (3×3).
// Fonction utilitaire pour simuler un affaissement local.
// -----------------------------------------------------------------------------
template<typename T>
void level(T& arrHeights, const std::size_t SIZEX, const std::size_t SIZEY, const int x, const int y)
{
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            if (x + i >= 0 && y + j >= 0 && x + i < static_cast<int>(SIZEX) && y + j < static_cast<int>(SIZEY)) {
                --arrHeights[x + i][y + j];
            }
        }
    }
}

// -----------------------------------------------------------------------------
// rivers
// Trouve le point le plus haut du terrain,
// lance un algorithme d’érosion ou de rivière,
// puis marque toutes les cases du chemin en -1.
// -----------------------------------------------------------------------------
template<typename T>
void rivers(T& arrHeights, const std::size_t SIZEX, const std::size_t SIZEY)
{
    // Recherche de la zone la plus haute.
    int x = 0, y = 0;
    for (int i = 0; i < SIZEX; ++i) {
        for (int j = 0; j < SIZEY; ++j) {
            if (arrHeights[i][j] > arrHeights[x][y]) {
                x = i;
                y = j;
            }
        }
    }
    mPondere.clear();
    std::vector<std::pair<int, int>> vPath;
    //_rivers(arrHeights, SIZEX, SIZEY, x, y, vPath);
    //_erosion(arrHeights, SIZEX, SIZEY, x, y, vPath);
    //_erosion_iter(arrHeights, SIZEX, SIZEY, x, y, vPath);
    _erosion_hydraulic_iter(arrHeights, SIZEX, SIZEY, x, y, vPath);
    for (const auto& ref : vPath) {
        arrHeights[ref.first][ref.second] = -1;
    }
    /*if (!mPondere.empty()) {
        for (const auto& ref : mPondere) {
            arrHeights[ref.first % SIZEY][ref.first / SIZEY] = -1;
        }
    }*/
}

// -----------------------------------------------------------------------------
// rivers_arrayed
// Trie toutes les cases par hauteur décroissante,
// lance _rivers depuis chaque maximum local,
// et marque les chemins en -1.
// S’arrête dès que la hauteur change.
// -----------------------------------------------------------------------------
template<typename T>
void rivers_arrayed(T& arrHeights, const std::size_t SIZEX, const std::size_t SIZEY)
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
