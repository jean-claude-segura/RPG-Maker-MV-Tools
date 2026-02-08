#pragma once

#include <map>
#include <vector>
#include <stack>

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

template<typename T>
bool _levelmin(T& arrHeights, const std::size_t SIZEX, const std::size_t SIZEY, const int x, const int y, std::vector<std::pair<int, int>>& vPath, std::tuple<int, int, int>& candidat)
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
    if (!_levelmin(arrHeights, SIZEX, SIZEY, x, y, vPath, firstcandidat))
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
            if (vPath.end() != std::find(vPath.begin(), vPath.end(), ref))
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
        std::vector<std::tuple<int, int, int>> temp;
        cstack.push(std::move(temp));
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
            if (x + i >= 0 && y + j >= 0 && x + i < static_cast<int>(SIZEX) && y + j < static_cast<int>(SIZEY)) {
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
