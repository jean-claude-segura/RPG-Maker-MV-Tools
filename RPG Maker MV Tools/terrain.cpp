#include "terrain.h"

// http://universe.tuxfamily.org/oldblog/index.php?post/2016/03/11/G%C3%A9n%C3%A9ration-proc%C3%A9durale-de-terrain-I
// https://github.com/DenisSalem/UNIVERSE/tree/master/PoC/improvedPerlinNoise
// http://universe.tuxfamily.org/oldblog/

// On d�finit une macro d'interpolation, � priori plus rapide qu'un appel de fonction.
// la formule est la suivante:  6t^5 - 15t^4 + 10t^3
//#define interpolation(t) (6 * t * t * t * t * t - 15 * t * t * t * t + 10 * t * t * t)
// <=> t^3 * 6t ^ 2 - t ^ 3 * 15t + t ^ 3 * 10
// <=> t^3 ( 6t^2 - 15t + 10)
// <=> (6*t*t-15*t+10)*t*t*t
// <=> ((6*t-15)*t+10)*t*t*t
#define interpolation(t) (((6*t-15)*t+10)*t*t*t)
//  Certaines variables et tableaux sont globaux afin de ne pas �tre empil�s lors de
//  l'appel de la fonction PerlinNoise2D, ce qui nous fera �conomiser des instructions
//  et du temps CPU.

// G est le tableau de gradient.
// Ici G comporte 8 paires de coordonn�es. Dans l'impl�mentation am�lior�e G comporte
// 12 triplets de coordonn�es pour pouvoir travailler dans un cube, et comme ce n'est pas
// le cas ici... :)
int G[8][2] = {
    {1,1},    {-1,1},   {1,-1},   {-1,-1},
    {1,0},    {-1,0},   {0,1},    {0,-1},
};

// La table de permutations. En l'�tat, l'algorithme produira toujours le m�me terrain 
// pseudo-al�atoire. Pour obtenir un vrai terrain pseudo-al�atoire diff�rent � chaque
// lancement du programme il faudrait changer ou d�sordonner cette table de permutations
// avant d'entrer dans la boucle principale o� est appel�e notre fonction de bruit.
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

// Les coordonn�es x et y doivent �tre des nombres r�els positifs, sinon nous serions en 
// dehors de notre grille. Pour cela on envoie � notre fonction les coordonn�es d'un
// point de la matrice, que l'on remet ensuite � l'�chelle de la fa�on suivante
// C = c / scale
// avec C la coordonn�e mise � l'�chelle et c la coordonn� enti�re d'origine.

// La fonction prend �galement en param�tre un entier -l'octave-
// positif qui d�termine la taille de notre grille. Par exemple si la matrice
// de destination � une r�solution de 256� �l�ments et que notre entier vaut
// 128 alors alors la taille de la grille sera en fait de 2� �l�ments, soit 3�
// noeuds :
//
//
//    <-- 256 --> taille r�elle de la matrix
//
//    0----1----2
//    |    |    |
//    3----1----5   Here stands the Grid. A digital frontiere...
//    |    |    |
//    6----7----8
//
//    <--- 2 ---> taille de la grille mise � l'�chelle

double terrain::PerlinNoise2D(const unsigned int x, const unsigned int y, const unsigned int scale) {

    // Chaque vecteur gradient est r�f�r� par un indice Gn du tableau G
    int G1, G2, G3, G4;

    // Contient le r�sultat du produit scalaire de Gn avec P-Q dans une base orthonormale.
    double s, t, u, v;

    //Contient les coordonn�es X et Y des points Qn et du point P.
    int Q1[2], Q2[2], Q3[2], Q4[2];
    double p[2];

    // R�sultat de la macro d'interpolation en X et Y, tmp permet de stocker P-Q avant d'�tre
    // inject� dans la macro.
    double iX, iY, tmp;

    // R�sultat de l'interpolation horizontal en st et uv
    double iST;
    double iUV;

    // L'initialisation peut sembler alambiqu�e, mais c'est 
    // pour �pargner au processeur des calculs inutiles.

    p[0] = static_cast<double>(x) / static_cast<double>(scale);
    p[1] = static_cast<double>(y) / static_cast<double>(scale);

    // Coin sup�rieur gauche
    Q1[0] = static_cast<int>(p[0]);
    Q1[1] = static_cast<int>(p[1]);

    // Coin sup�rieur droit
    Q2[0] = Q1[0] + 1;
    Q2[1] = Q1[1];

    // Coin inf�rieur droit
    Q3[0] = Q2[0];
    Q3[1] = Q1[1] + 1;

    // Coin inf�rieur gauche
    Q4[0] = Q1[0];
    Q4[1] = Q3[1];


    // On r�cup�re pseudo al�atoirement les gradients.
    // Pour �viter la r�p�tition de motifs fractals � chaque octave 
    // l'indice final d�pend de l'octave courant
    G1 = P[(Q1[0] + P[Q1[1] & 255] + scale) & 255] & 7;  // Gradient sup�rieur gauche
    G2 = P[(Q2[0] + P[Q2[1] & 255] + scale) & 255] & 7;  // Gradient sup�rieur droit
    G3 = P[(Q3[0] + P[Q3[1] & 255] + scale) & 255] & 7;  // Gradient inf�rieur droit
    G4 = P[(Q4[0] + P[Q4[1] & 255] + scale) & 255] & 7;  // Gradient inf�rieur gauche

    // On calcule le produit scalaire Gn . (P-Qn)
    // Avec P faisant r�f�rence aux coordonn�es du point stock� dans p.
    // (P �tant la table de permutations)
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

    // On retourne le r�sultat normalis�.
    return (1 + iST + iY * (iUV - iST)) * 0.5;
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
                double X = abs(x - i);
                double Y = abs(y - j);
#ifdef _DEBUG
                matrix[i][j] += _sinc[X][Y];
#else
                matrix[i][j] = matrix[i][j] + _sinc[X][Y];
#endif
            }
        }
    }
}
