#include "opensimplex2.h"

const double OpenSimplex2S::GRADIENTS_2D[16][2] = {
    { 0.1305261922,  0.9914448614}, { 0.3826834324,  0.9238795325},
    { 0.6087614290,  0.7933533403}, { 0.7933533403,  0.6087614290},
    { 0.9238795325,  0.3826834324}, { 0.9914448614,  0.1305261922},
    { 0.9914448614, -0.1305261922}, { 0.9238795325, -0.3826834324},
    { 0.7933533403, -0.6087614290}, { 0.6087614290, -0.7933533403},
    { 0.3826834324, -0.9238795325}, { 0.1305261922, -0.9914448614},
    {-0.1305261922, -0.9914448614}, {-0.3826834324, -0.9238795325},
    {-0.6087614290, -0.7933533403}, {-0.7933533403, -0.6087614290}
};

OpenSimplex2S::OpenSimplex2S(int seed) {
    std::mt19937 rng(seed);
    std::array<int, 256> p;

    for (int i = 0; i < 256; i++) p[i] = i;
    std::shuffle(p.begin(), p.end(), rng);

    for (int i = 0; i < 256; i++) {
        perm[i] = p[i];
        permGrad2[i] = p[i] % GRADIENTS_2D_COUNT;
    }
}

double OpenSimplex2S::noise2(double x, double y) {
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
