// Physical constants and the parameter set of a Foucault pendulum run.
#pragma once

#include <cmath>
#include <stdexcept>
#include <string>

namespace foucault {

inline constexpr double kPi = 3.14159265358979323846;

// Length of one rotation of the Earth against the fixed stars, in SI seconds.
// This is the period that sets the Coriolis effect (the solar day, 86400 s,
// includes the extra turn the Earth makes to follow the Sun).
inline constexpr double kSiderealDay = 86164.0905;

// The value hard-coded in the 2021 course code. It is a typo for 86400 s and
// is kept only so that the original output files can be reproduced.
inline constexpr double kLegacyDay = 84600.0;

inline constexpr double kDegToRad = kPi / 180.0;

enum class Model {
    Simplified,  // Coriolis coupling only
    Full,        // adds the centrifugal (Omega^2) terms of the course report
};

inline const char* to_string(Model m) { return m == Model::Full ? "full" : "simplified"; }

struct Params {
    double gravity = 9.83;           // m/s^2
    double length = 67.0;            // m, pivot to bob centre
    double latitude_deg = 49.0;      // degrees, north positive
    double amplitude = -1.0;         // m, release distance along +x; <= 0 means length / 100
    double day = kSiderealDay;       // s, rotation period of the Earth
    double omega_scale = 1.0;        // multiplies Omega, to exaggerate the effect
    double dt = 0.1;                 // s, time step
    Model model = Model::Simplified;

    double release() const { return amplitude > 0.0 ? amplitude : length / 100.0; }
    double omega() const { return omega_scale * 2.0 * kPi / day; }
    double latitude() const { return latitude_deg * kDegToRad; }

    // Angular frequency of the swing, sqrt(g / L).
    double swing_omega() const { return std::sqrt(gravity / length); }
    double swing_period() const { return 2.0 * kPi / swing_omega(); }

    // Rate at which the swing plane turns, Omega sin(latitude). Positive means
    // clockwise seen from above in the northern hemisphere.
    double precession_rate() const { return omega() * std::sin(latitude()); }

    // Time for the swing plane to turn once, 2 pi / (Omega sin(latitude)).
    // Infinite at the equator.
    double precession_period() const {
        const double rate = std::fabs(precession_rate());
        return rate == 0.0 ? INFINITY : 2.0 * kPi / rate;
    }

    void validate() const {
        auto require = [](bool ok, const char* what) {
            if (!ok) throw std::invalid_argument(what);
        };
        require(std::isfinite(gravity) && gravity > 0.0, "gravity must be positive");
        require(std::isfinite(length) && length > 0.0, "length must be positive");
        require(std::isfinite(latitude_deg) && latitude_deg >= -90.0 && latitude_deg <= 90.0,
                "latitude must be within [-90, 90] degrees");
        require(std::isfinite(day) && day > 0.0, "day length must be positive");
        require(std::isfinite(omega_scale) && omega_scale >= 0.0, "omega scale must be >= 0");
        require(std::isfinite(dt) && dt > 0.0, "time step must be positive");
        require(std::isfinite(amplitude), "amplitude must be finite");
        // Small-angle model: the bob must stay close to the vertical.
        require(release() < 0.2 * length, "release distance must be below 20% of the length");
    }
};

}  // namespace foucault
