// A tiny assertion helper so the tests need no third-party framework.
#pragma once

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace check {

inline int& failures() {
    static int n = 0;
    return n;
}

inline void expect(bool ok, const char* what, const char* file, int line) {
    if (!ok) {
        std::fprintf(stderr, "FAIL %s:%d  %s\n", file, line, what);
        ++failures();
    }
}

inline void near(double got, double want, double tol, const char* what, const char* file, int line) {
    if (!(std::fabs(got - want) <= tol)) {
        std::fprintf(stderr, "FAIL %s:%d  %s: got %.12g, want %.12g (tol %.3g)\n", file, line, what, got, want, tol);
        ++failures();
    }
}

inline int finish(const char* name) {
    if (failures() == 0) {
        std::printf("%s: all checks passed\n", name);
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "%s: %d check(s) failed\n", name, failures());
    return EXIT_FAILURE;
}

}  // namespace check

#define EXPECT(cond) ::check::expect((cond), #cond, __FILE__, __LINE__)
#define EXPECT_NEAR(got, want, tol) ::check::near((got), (want), (tol), #got, __FILE__, __LINE__)
