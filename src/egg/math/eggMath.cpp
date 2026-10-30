// TODO(kiwi) REMOVE AFTER REFACTOR
#pragma ipa file

#include <egg/math.h>
#include <egg/prim.h>

#include <cmath>

namespace EGG {

template <typename T> T Math<T>::sqrt(T t) {
#line 14
    EGG_ASSERT(t >= 0);
    return std::sqrt(t);
}

template <typename T> T Math<T>::sin(T t) {
    return std::sin(t);
}

template <typename T> T Math<T>::cos(T t) {
    return std::cos(t);
}

template <typename T> T Math<T>::asin(T t) {
#line 41
    EGG_ASSERT(-1 <= t && t <= 1);
    return std::asin(t);
}

template <typename T> T Math<T>::acos(T t) {
#line 48
    EGG_ASSERT(-1 <= t && t <= 1);
    return std::acos(t);
}

template <typename T> T Math<T>::atan2(T ty, T tx) {
    return std::atan2(ty, tx);
}

template class Math<f32>;

// Unreferenced in Wii Play (stripped by the linker); only its print string survives.
int gcd(int m, int n) {
    EGG_PRINT("gcd(%d,%d)\n", m, n);

    while (n != 0) {
        int r = m % n;
        m = n;
        n = r;
    }

    return m;
}

} // namespace EGG
