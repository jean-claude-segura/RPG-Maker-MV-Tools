#include "misc_landscape.h"

#ifdef _DEBUG
void thrSinusCard(std::unique_ptr<std::unique_ptr<double[]>[]>& matrix, std::unique_ptr<std::unique_ptr<double[]>[]>& _sinc, const unsigned int scale)
#else
void terrain::thrSinusCard(std::unique_ptr<std::unique_ptr<std::atomic<double>[]>[]>& matrix, std::unique_ptr<std::unique_ptr<double[]>[]>& _sinc, const unsigned int scale)
#endif 
{
    for (int iter = 0; iter < 250; ++iter) {
        double x, y;
        double K = terrain_base::random_between(0., static_cast<double>(scale) / 2.);
        double L = terrain_base::random_between(0., 100.);
        x = terrain_base::random_between(0., static_cast<double>(scale));
        y = terrain_base::random_between(0., static_cast<double>(scale));
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
