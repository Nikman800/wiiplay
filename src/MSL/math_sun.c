#include <math.h>

double scalbn(double x, int n) {
    int e;

    x = frexp(x, &e);
    e += n;
    return ldexp(x, e);
}
