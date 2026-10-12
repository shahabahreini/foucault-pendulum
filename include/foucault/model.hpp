// The Foucault pendulum in the horizontal plane, small-angle approximation.
//
// In the frame that turns with the Earth (rate Omega, latitude lambda) the bob
// feels the Coriolis force -2 m Omega x v. Only the vertical part of Omega,
// Omega sin(lambda), acts on horizontal motion, which gives
//
//   x'' - 2 Omega sin(lambda) y' + (g/L) x = 0
//   y'' + 2 Omega sin(lambda) x' + (g/L) y = 0
//
// The "full" model of the course report adds the centrifugal terms:
//
//   x'' - 2 Omega sin(lambda) y' + (g/L - Omega^2 sin^2(lambda)) x = 0
//   y'' + 2 Omega sin(lambda) x' + (g/L - Omega^2)               y = 0
//
// The mass cancels and there is no damping.
#pragma once

#include <cmath>
#include <complex>

#include "foucault/params.hpp"
#include "foucault/rk4.hpp"

namespace foucault {

struct State {
    double x = 0.0, y = 0.0;    // m
    double vx = 0.0, vy = 0.0;  // m/s
};

inline State operator+(const State& a, const State& b) {
    return {a.x + b.x, a.y + b.y, a.vx + b.vx, a.vy + b.vy};
}
inline State operator*(double s, const State& a) { return {s * a.x, s * a.y, s * a.vx, s * a.vy}; }

class Pendulum {
public:
    explicit Pendulum(const Params& p) : p_(p) {
        p_.validate();
        const double omega = p_.omega();
        const double sin_lat = std::sin(p_.latitude());
        const double w0sq = p_.gravity / p_.length;
        coriolis_ = 2.0 * omega * sin_lat;
        if (p_.model == Model::Full) {
            kx_ = w0sq - omega * omega * sin_lat * sin_lat;
            ky_ = w0sq - omega * omega;
        } else {
            kx_ = ky_ = w0sq;
        }
    }

    const Params& params() const { return p_; }

    // Time derivative of the state: (vx, vy, ax, ay).
    State rhs(const State& s) const {
        return {s.vx, s.vy, -kx_ * s.x + coriolis_ * s.vy, -ky_ * s.y - coriolis_ * s.vx};
    }

    State initial() const { return {p_.release(), 0.0, 0.0, 0.0}; }

    State step(const State& s) const { return step(s, p_.dt); }

    State step(const State& s, double dt) const {
        return rk4_step([this](const State& q) { return rhs(q); }, s, dt);
    }

    // Exact solution of the simplified model for release from rest at x = a:
    //   zeta = x + i y = a exp(-i Omega sin(lambda) t) [cos(w t) + i (Omega sin(lambda) / w) sin(w t)]
    // with w = sqrt(g/L + (Omega sin(lambda))^2).
    std::complex<double> exact_simplified(double t) const {
        const double a = p_.release();
        const double s = p_.precession_rate();
        const double w = std::sqrt(p_.gravity / p_.length + s * s);
        using C = std::complex<double>;
        return a * std::exp(C(0.0, -s * t)) * C(std::cos(w * t), (s / w) * std::sin(w * t));
    }

private:
    Params p_;
    double coriolis_ = 0.0, kx_ = 0.0, ky_ = 0.0;
};

}  // namespace foucault
