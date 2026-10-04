//------------------------------------------------------------------------------
// Shared zero-dependency check macros for the unit_tests TUs.
// g_checks/g_failures are defined once in unit_tests.cpp.
//------------------------------------------------------------------------------
#pragma once
#include <cstdio>

extern int g_checks;
extern int g_failures;

#define CHECK(cond)                                                        \
    do {                                                                   \
        ++g_checks;                                                        \
        if (!(cond)) {                                                     \
            ++g_failures;                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);    \
        }                                                                  \
    } while (0)

#define CHECK_EQ(a, b)                                                     \
    do {                                                                   \
        ++g_checks;                                                        \
        long long _va = (long long)(a), _vb = (long long)(b);             \
        if (_va != _vb) {                                                  \
            ++g_failures;                                                  \
            std::printf("FAIL %s:%d: %s (=%lld) != %s (=%lld)\n",          \
                        __FILE__, __LINE__, #a, _va, #b, _vb);             \
        }                                                                  \
    } while (0)

#define CHECK_FEQ(a, b)                                                    \
    do {                                                                   \
        ++g_checks;                                                        \
        double _va = (double)(a), _vb = (double)(b);                      \
        double _d = _va - _vb;                                             \
        if (_d < -1e-6 || _d > 1e-6) {                                     \
            ++g_failures;                                                  \
            std::printf("FAIL %s:%d: %s (=%g) != %s (=%g)\n",              \
                        __FILE__, __LINE__, #a, _va, #b, _vb);             \
        }                                                                  \
    } while (0)

#define CHECK_NEAR(a, b, eps)                                              \
    do {                                                                   \
        ++g_checks;                                                        \
        double _va = (double)(a), _vb = (double)(b), _e = (eps);          \
        double _d = _va - _vb;                                             \
        if (_d < -_e || _d > _e) {                                         \
            ++g_failures;                                                  \
            std::printf("FAIL %s:%d: %s (=%g) != %s (=%g) eps=%g\n",       \
                        __FILE__, __LINE__, #a, _va, #b, _vb, _e);         \
        }                                                                  \
    } while (0)
