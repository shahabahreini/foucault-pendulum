// Physics checks for the Foucault pendulum solver.

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "check.hpp"
#include "foucault/model.hpp"
#include "foucault/params.hpp"

using namespace foucault;

namespace {

struct Sample {
    double t, x, y;
};

std::vector<Sample> run(const Params& p, double duration) {
    const Pendulum pendulum(p);
    const long steps = std::lround(duration / p.dt);
    std::vector<Sample> out;
    out.reserve(static_cast<std::size_t>(steps) + 1);
    State s = pendulum.initial();
    for (long i = 0; i <= steps; ++i) {
        out.push_back({static_cast<double>(i) * p.dt, s.x, s.y});
        s = pendulum.step(s);
    }
    return out;
}

double max_error_vs_exact(const Params& p, double duration) {
    const Pendulum pendulum(p);
    double worst = 0.0;
    for (const Sample& s : run(p, duration)) {
        worst = std::max(worst, std::abs(std::complex<double>(s.x, s.y) - pendulum.exact_simplified(s.t)));
    }
    return worst;
}

// Angle of the swing line at the last turning point before `t_end`, taken from the samples.
double plane_angle_at_end(const std::vector<Sample>& samples) {
    for (std::size_t i = samples.size() - 2; i > 0; --i) {
        const double r_prev = std::hypot(samples[i - 1].x, samples[i - 1].y);
        const double r = std::hypot(samples[i].x, samples[i].y);
        const double r_next = std::hypot(samples[i + 1].x, samples[i + 1].y);
        if (r > r_prev && r >= r_next) return std::atan2(samples[i].y, samples[i].x);
    }
    return 0.0;
}

// Difference of two angles of a line (period pi), in (-pi/2, pi/2].
double line_angle_difference(double a, double b) {
    double d = std::fmod(a - b, kPi);
    if (d > kPi / 2) d -= kPi;
    if (d <= -kPi / 2) d += kPi;
    return d;
}

void test_matches_exact_solution() {
    Params p;
    p.dt = 0.1;
    // One hour at dt = 0.1 s: RK4 and the closed form agree to better than 0.05 mm.
    // The remaining error is a slow drift in the swing phase, about 17 micrometres per hour.
    EXPECT(max_error_vs_exact(p, 3600.0) < 5e-5);
}

void test_fourth_order_convergence() {
    Params p;
    double previous = 0.0;
    for (double dt : {1.0, 0.5, 0.25}) {
        p.dt = dt;
        const double err = max_error_vs_exact(p, 600.0);
        if (previous > 0.0) {
            const double ratio = previous / err;
            // Fourth order halves dt and cuts the error by 2^4 = 16.
            EXPECT(ratio > 14.0 && ratio < 18.0);
        }
        previous = err;
    }
}

void test_precession_period(double latitude_deg) {
    Params p;
    p.latitude_deg = latitude_deg;
    p.dt = 0.1;
    const double period = p.precession_period();
    const double duration = std::isfinite(period) ? period : 6.0 * 3600.0;
    const std::vector<Sample> samples = run(p, duration);
    const double got = plane_angle_at_end(samples);
    const double want = -p.precession_rate() * samples.back().t;
    // The turning point is sampled every 0.1 s; allow 0.05 degrees.
    const double diff = line_angle_difference(got, want);
    EXPECT_NEAR(diff, 0.0, 0.05 * kDegToRad);
}

void test_precession_sense_and_equator() {
    Params north, south, equator;
    north.latitude_deg = 49.0;
    south.latitude_deg = -49.0;
    equator.latitude_deg = 0.0;
    // Seen from above the swing plane turns clockwise in the north (negative angle), the other way in the south.
    EXPECT(line_angle_difference(plane_angle_at_end(run(north, 7200.0)), 0.0) < 0.0);
    EXPECT(line_angle_difference(plane_angle_at_end(run(south, 7200.0)), 0.0) > 0.0);
    // At the equator nothing turns and y stays zero.
    double worst_y = 0.0;
    for (const Sample& s : run(equator, 3600.0)) worst_y = std::max(worst_y, std::fabs(s.y));
    EXPECT(worst_y < 1e-12);
}

void test_known_periods() {
    Params p;
    p.latitude_deg = 90.0;
    EXPECT_NEAR(p.precession_period(), kSiderealDay, 1e-6);
    p.latitude_deg = 49.0;
    EXPECT_NEAR(p.precession_period() / 3600.0, kSiderealDay / std::sin(49.0 * kDegToRad) / 3600.0, 1e-9);
    p.latitude_deg = 0.0;
    EXPECT(std::isinf(p.precession_period()));
    Params d;  // swing period of a 67 m pendulum
    EXPECT_NEAR(d.swing_period(), 16.4, 0.05);
}

void test_models_agree_at_earth_rate() {
    Params simple, full;
    full.model = Model::Full;
    const std::vector<Sample> a = run(simple, 3500.0);
    const std::vector<Sample> b = run(full, 3500.0);
    double worst = 0.0;
    for (std::size_t i = 0; i < a.size(); ++i) worst = std::max(worst, std::hypot(a[i].x - b[i].x, a[i].y - b[i].y));
    EXPECT(worst < 5e-5);

    // With Omega scaled up by 300 the centrifugal terms matter, as in the course report.
    simple.omega_scale = full.omega_scale = 300.0;
    const std::vector<Sample> c = run(simple, 3500.0);
    const std::vector<Sample> d = run(full, 3500.0);
    worst = 0.0;
    for (std::size_t i = 0; i < c.size(); ++i) worst = std::max(worst, std::hypot(c[i].x - d[i].x, c[i].y - d[i].y));
    EXPECT(worst > 5e-3);
}

std::vector<Sample> read_reference(const std::string& path) {
    std::ifstream in(path);
    std::vector<Sample> rows;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream fields(line);
        Sample s{};
        if (fields >> s.t >> s.x >> s.y) rows.push_back(s);
    }
    return rows;
}

void test_reproduces_2021_output(Model model, const char* file) {
    Params p;
    p.day = kLegacyDay;  // the 2021 code used 84600 s
    p.model = model;
    const std::vector<Sample> want = read_reference(std::string(FOUCAULT_REFERENCE_DIR) + "/" + file);
    EXPECT(want.size() == 3500);
    const std::vector<Sample> got = run(p, 3499.0);
    double worst = 0.0;
    for (const Sample& w : want) {
        const std::size_t i = static_cast<std::size_t>(std::lround(w.t / p.dt));
        if (i >= got.size()) {
            EXPECT(false);
            break;
        }
        worst = std::max(worst, std::max(std::fabs(got[i].x - w.x), std::fabs(got[i].y - w.y)));
    }
    // The 2021 files carry six significant digits.
    EXPECT(worst < 2e-6);
}

void test_validation() {
    auto rejects = [](Params p) {
        try {
            Pendulum q(p);
            (void)q;
        } catch (const std::invalid_argument&) {
            return true;
        }
        return false;
    };
    Params p;
    p.latitude_deg = 91.0;
    EXPECT(rejects(p));
    p = Params{};
    p.length = 0.0;
    EXPECT(rejects(p));
    p = Params{};
    p.dt = -0.1;
    EXPECT(rejects(p));
    p = Params{};
    p.amplitude = 30.0;  // far from the small-angle regime
    EXPECT(rejects(p));
}

}  // namespace

int main() {
    test_matches_exact_solution();
    test_fourth_order_convergence();
    for (double lat : {10.0, 49.0, 90.0}) test_precession_period(lat);
    test_precession_sense_and_equator();
    test_known_periods();
    test_models_agree_at_earth_rate();
    test_reproduces_2021_output(Model::Simplified, "legacy-simplified.tsv");
    test_reproduces_2021_output(Model::Full, "legacy-full.tsv");
    test_validation();
    return check::finish("test_physics");
}
