#include "terrain.h"

// http://universe.tuxfamily.org/oldblog/index.php?post/2016/03/11/G%C3%A9n%C3%A9ration-proc%C3%A9durale-de-terrain-I
// https://github.com/DenisSalem/UNIVERSE/tree/master/PoC/improvedPerlinNoise
// http://universe.tuxfamily.org/oldblog/

// On définit une macro d'interpolation, à priori plus rapide qu'un appel de fonction.
// la formule est la suivante:  6t^5 - 15t^4 + 10t^3
//#define interpolation(t) (6 * t * t * t * t * t - 15 * t * t * t * t + 10 * t * t * t)
// <=> t^3 * 6t ^ 2 - t ^ 3 * 15t + t ^ 3 * 10
// <=> t^3 ( 6t^2 - 15t + 10)
// <=> (6*t*t-15*t+10)*t*t*t
// <=> ((6*t-15)*t+10)*t*t*t
#define interpolation(t) (((6*t-15)*t+10)*t*t*t)
//  Certaines variables et tableaux sont globaux afin de ne pas être empilés lors de
//  l'appel de la fonction PerlinNoise2D, ce qui nous fera économiser des instructions
//  et du temps CPU.

// G est le tableau de gradient.
// Ici G comporte 8 paires de coordonnées. Dans l'implémentation améliorée G comporte
// 12 triplets de coordonnées pour pouvoir travailler dans un cube, et comme ce n'est pas
// le cas ici... :)
int G[8][2] = {
    {1,1},    {-1,1},   {1,-1},   {-1,-1},
    {1,0},    {-1,0},   {0,1},    {0,-1},
};

// La table de permutations. En l'état, l'algorithme produira toujours le même terrain 
// pseudo-aléatoire. Pour obtenir un vrai terrain pseudo-aléatoire différent à chaque
// lancement du programme il faudrait changer ou désordonner cette table de permutations
// avant d'entrer dans la boucle principale où est appelée notre fonction de bruit.
int terrain::POrg[256] = {
 235,249,14,239,107,49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,
 127, 4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,
 112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,
 153,101,155,167, 43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185,
 74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,
 187,208, 89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,
 47,16,58,17,182,189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,
 64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,
 203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,
 105,92,41,55,46,245,40,244,102,143,54, 65,25,63,161,1,216,80,73,209,76,132,
 142,8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,
 151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,
 156,180 };

int terrain::P[256] = {};

// Les coordonnées x et y doivent être des nombres réels positifs, sinon nous serions en 
// dehors de notre grille. Pour cela on envoie à notre fonction les coordonnées d'un
// point de la matrice, que l'on remet ensuite à l'échelle de la façon suivante
// C = c / scale
// avec C la coordonnée mise à l'échelle et c la coordonné entière d'origine.

// La fonction prend également en paramètre un entier -l'octave-
// positif qui détermine la taille de notre grille. Par exemple si la matrice
// de destination à une résolution de 256² éléments et que notre entier vaut
// 128 alors alors la taille de la grille sera en fait de 2² éléments, soit 3²
// noeuds :
//
//
//    <-- 256 --> taille réelle de la matrix
//
//    0----1----2
//    |    |    |
//    3----1----5   Here stands the Grid. A digital frontiere...
//    |    |    |
//    6----7----8
//
//    <--- 2 ---> taille de la grille mise à l'échelle

double terrain::PerlinNoise2D(const unsigned int x, const unsigned int y, const unsigned int scale) {

    // Chaque vecteur gradient est référé par un indice Gn du tableau G
    int G1, G2, G3, G4;

    // Contient le résultat du produit scalaire de Gn avec P-Q dans une base orthonormale.
    double s, t, u, v;

    //Contient les coordonnées X et Y des points Qn et du point P.
    int Q1[2], Q2[2], Q3[2], Q4[2];
    double p[2];

    // Résultat de la macro d'interpolation en X et Y, tmp permet de stocker P-Q avant d'être
    // injecté dans la macro.
    double iX, iY, tmp;

    // Résultat de l'interpolation horizontal en st et uv
    double iST;
    double iUV;

    // L'initialisation peut sembler alambiquée, mais c'est 
    // pour épargner au processeur des calculs inutiles.

    p[0] = static_cast<double>(x) / static_cast<double>(scale);
    p[1] = static_cast<double>(y) / static_cast<double>(scale);

    // Coin supérieur gauche
    Q1[0] = static_cast<int>(p[0]);
    Q1[1] = static_cast<int>(p[1]);

    // Coin supérieur droit
    Q2[0] = Q1[0] + 1;
    Q2[1] = Q1[1];

    // Coin inférieur droit
    Q3[0] = Q2[0];
    Q3[1] = Q1[1] + 1;

    // Coin inférieur gauche
    Q4[0] = Q1[0];
    Q4[1] = Q3[1];


    // On récupére pseudo aléatoirement les gradients.
    // Pour éviter la répétition de motifs fractals à chaque octave 
    // l'indice final dépend de l'octave courant
    G1 = P[(Q1[0] + P[Q1[1] & 255] + scale) & 255] & 7;  // Gradient supérieur gauche
    G2 = P[(Q2[0] + P[Q2[1] & 255] + scale) & 255] & 7;  // Gradient supérieur droit
    G3 = P[(Q3[0] + P[Q3[1] & 255] + scale) & 255] & 7;  // Gradient inférieur droit
    G4 = P[(Q4[0] + P[Q4[1] & 255] + scale) & 255] & 7;  // Gradient inférieur gauche

    // On calcule le produit scalaire Gn . (P-Qn)
    // Avec P faisant référence aux coordonnées du point stocké dans p.
    // (P étant la table de permutations)
    s = G[G1][0] * (p[0] - Q1[0]) + G[G1][1] * (p[1] - Q1[1]);
    t = G[G2][0] * (p[0] - Q2[0]) + G[G2][1] * (p[1] - Q2[1]);
    u = G[G3][0] * (p[0] - Q3[0]) + G[G3][1] * (p[1] - Q3[1]);
    v = G[G4][0] * (p[0] - Q4[0]) + G[G4][1] * (p[1] - Q4[1]);


    // On interpole en x sur le segment st et uv
    tmp = p[0] - Q1[0];
    iX = interpolation(tmp);

    iST = s + iX * (t - s);
    iUV = v + iX * (u - v);

    // On interpole y sur le segment iST et iUV
    tmp = p[1] - Q1[1];

    iY = interpolation(tmp);

    // On retourne le résultat normalisé.
    return (1 + iST + iY * (iUV - iST)) * 0.5;
}

double terrain::simplex2d(double x, double y) {
    const double F2 = 0.5 * (sqrt(3.0) - 1.0);
    const double G2 = (3.0 - sqrt(3.0)) / 6.0;

    double s = (x + y) * F2;
    int i = fastfloor(x + s);
    int j = fastfloor(y + s);

    double t = (i + j) * G2;
    double X0 = i - t;
    double Y0 = j - t;
    double x0 = x - X0;
    double y0 = y - Y0;

    int i1, j1;
    if (x0 > y0) { i1 = 1; j1 = 0; }
    else { i1 = 0; j1 = 1; }

    double x1 = x0 - i1 + G2;
    double y1 = y0 - j1 + G2;
    double x2 = x0 - 1.0 + 2.0 * G2;
    double y2 = y0 - 1.0 + 2.0 * G2;

    int ii = i & 255;
    int jj = j & 255;

    int gi0 = perm[ii + perm[jj]] & 7;
    int gi1 = perm[ii + i1 + perm[jj + j1]] & 7;
    int gi2 = perm[ii + 1 + perm[jj + 1]] & 7;

    double n0, n1, n2;

    double t0 = 0.5 - x0 * x0 - y0 * y0;
    if (t0 < 0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad2[gi0], x0, y0);
    }

    double t1 = 0.5 - x1 * x1 - y1 * y1;
    if (t1 < 0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad2[gi1], x1, y1);
    }

    double t2 = 0.5 - x2 * x2 - y2 * y2;
    if (t2 < 0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad2[gi2], x2, y2);
    }

    return 70.0 * (n0 + n1 + n2); // ? [-1,1]
}

double terrain::simplex_2d(double x, double y, int seed) {
    buildPermutation(seed);
    return simplex2d(x, y);
}

#ifdef _DEBUG
void terrain::thrSinusCard(std::unique_ptr<std::unique_ptr<double[]>[]>& matrix, std::unique_ptr<std::unique_ptr<double[]>[]>& _sinc, const unsigned int scale)
#else
void terrain::thrSinusCard(std::unique_ptr<std::unique_ptr<std::atomic<double>[]>[]>& matrix, std::unique_ptr<std::unique_ptr<double[]>[]>& _sinc, const unsigned int scale)
#endif 
{
    for (int iter = 0; iter < 250; ++iter) {
        double x, y;
        double K = random_between(0., static_cast<double>(scale) / 2.);
        double L = random_between(0., 100.);
        x = random_between(0., static_cast<double>(scale));
        y = random_between(0., static_cast<double>(scale));
        for (int i = 0; i < scale; ++i) {
            for (int j = 0; j < scale; ++j) {
                /*double X = x - i;
                double Y = y - j;
                double Z = sqrt(X * X + Y * Y);
                double h = sinc(Z);
                matrix[i][j] += h;*/
                //double X = abs(x - i);
                //double Y = abs(y - j);
                int X = std::min<int>(abs(x - i), scale - 1);
                int Y = std::min<int>(abs(y - j), scale - 1);
#ifdef _DEBUG
                matrix[i][j] += _sinc[X][Y];
#else
                matrix[i][j] = matrix[i][j] + _sinc[X][Y];
#endif
            }
        }
    }
}

const double terrain::OpenSimplex2S::GRADIENTS_2D[16][2] = {
    { 0.1305261922,  0.9914448614}, { 0.3826834324,  0.9238795325},
    { 0.6087614290,  0.7933533403}, { 0.7933533403,  0.6087614290},
    { 0.9238795325,  0.3826834324}, { 0.9914448614,  0.1305261922},
    { 0.9914448614, -0.1305261922}, { 0.9238795325, -0.3826834324},
    { 0.7933533403, -0.6087614290}, { 0.6087614290, -0.7933533403},
    { 0.3826834324, -0.9238795325}, { 0.1305261922, -0.9914448614},
    {-0.1305261922, -0.9914448614}, {-0.3826834324, -0.9238795325},
    {-0.6087614290, -0.7933533403}, {-0.7933533403, -0.6087614290}
};

terrain::OpenSimplex2S::OpenSimplex2S(int seed) {
    std::mt19937 rng(seed);
    std::array<int, 256> p;

    for (int i = 0; i < 256; i++) p[i] = i;
    std::shuffle(p.begin(), p.end(), rng);

    for (int i = 0; i < 256; i++) {
        perm[i] = p[i];
        permGrad2[i] = p[i] % GRADIENTS_2D_COUNT;
    }
}

double terrain::OpenSimplex2S::noise2(double x, double y) {
    // Rotation du domaine (clé de OpenSimplex2S)
    const double R = 0.5 * (std::sqrt(3.0) - 1.0);
    double s = (x + y) * R;
    double xs = x + s;
    double ys = y + s;

    int xsb = fastfloor(xs);
    int ysb = fastfloor(ys);

    double xsi = xs - xsb;
    double ysi = ys - ysb;

    double t = (xsi + ysi) * (1.0 / 3.0);
    double xr = xsi - t;
    double yr = ysi - t;

    double value = 0.0;

    // Contributions
    for (int i = 0; i < 3; i++) {
        int dx = (i == 0) ? 0 : (xsi > ysi ? 1 : 0);
        int dy = (i == 0) ? 0 : (xsi > ysi ? 0 : 1);

        if (i == 2) { dx = 1; dy = 1; }

        double dxr = xr - dx + i * (1.0 / 3.0);
        double dyr = yr - dy + i * (1.0 / 3.0);

        double attn = 0.5 - dxr * dxr - dyr * dyr;
        if (attn > 0) {
            int px = (xsb + dx) & 255;
            int py = (ysb + dy) & 255;

            int gi = permGrad2[(perm[px] + py) & 255];
            const double* g = GRADIENTS_2D[gi];

            double dot = g[0] * dxr + g[1] * dyr;

            attn *= attn;
            value += attn * attn * dot;
        }
    }

    return value * 99.20689070704672; // Normalisation officielle
}
