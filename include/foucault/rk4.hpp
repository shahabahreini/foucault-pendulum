// A classical fourth-order Runge-Kutta step for any state with + and * by a scalar.
#pragma once

namespace foucault {

// y_{n+1} = y_n + dt/6 (k1 + 2 k2 + 2 k3 + k4), with k_i the slope of the
// right-hand side `f` at the four standard stages.
template <class State, class Rhs>
State rk4_step(const Rhs& f, const State& y, double dt) {
    const State k1 = f(y);
    const State k2 = f(y + (0.5 * dt) * k1);
    const State k3 = f(y + (0.5 * dt) * k2);
    const State k4 = f(y + dt * k3);
    return y + (dt / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
}

}  // namespace foucault
