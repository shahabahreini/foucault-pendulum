// foucault-sim: integrate a Foucault pendulum with fourth-order Runge-Kutta
// and write its horizontal trajectory as tab-separated text.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <string>

#include "foucault/model.hpp"
#include "foucault/params.hpp"

namespace {

constexpr const char* kVersion = "1.0.0";

constexpr const char* kUsage = R"(foucault-sim - Foucault pendulum, fourth-order Runge-Kutta

usage: foucault-sim [options]

physics
  --lat DEG          latitude in degrees, north positive        (49)
  --length M         pendulum length in metres                  (67)
  --gravity G        gravitational acceleration, m/s^2          (9.83)
  --amplitude M      release distance on x, metres              (length / 100)
  --day S            rotation period of the Earth, seconds      (86164.0905, sidereal)
  --legacy-day       use 84600 s, the value in the 2021 course code
  --omega-scale K    multiply Omega by K to exaggerate the effect (1)
  --model NAME       simplified | full                          (simplified)

numerics
  --dt S             time step, seconds                         (0.1)
  --duration S       simulated time, seconds                    (3500)
  --hours H          simulated time, hours (overrides --duration)
  --stride N         write every Nth step                       (1)

output
  --out FILE         write to FILE ('-' for standard output)    (-)
  --precision N      significant digits                         (12)
  --velocities       add the columns vx and vy
  --quiet            do not print the summary on standard error
  --help, --version

output columns: t[s]  x[m]  y[m]  (then vx[m/s] vy[m/s] with --velocities)
)";

[[noreturn]] void fail(const std::string& message) {
    std::fprintf(stderr, "foucault-sim: %s\n(try --help)\n", message.c_str());
    std::exit(2);
}

double to_double(const char* option, const char* text) {
    char* end = nullptr;
    const double v = std::strtod(text, &end);
    if (end == text || *end != '\0' || !std::isfinite(v)) {
        fail(std::string("invalid number for ") + option + ": '" + text + "'");
    }
    return v;
}

long to_long(const char* option, const char* text) {
    char* end = nullptr;
    const long v = std::strtol(text, &end, 10);
    if (end == text || *end != '\0' || v < 1) {
        fail(std::string("invalid positive integer for ") + option + ": '" + text + "'");
    }
    return v;
}

struct Options {
    foucault::Params params;
    double duration = 3500.0;
    long stride = 1;
    int precision = 12;
    bool velocities = false;
    bool quiet = false;
    std::string out = "-";
};

Options parse(int argc, char** argv) {
    Options o;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        auto value = [&]() -> const char* {
            if (i + 1 >= argc) fail("missing value for " + a);
            return argv[++i];
        };
        if (a == "--help" || a == "-h") {
            std::fputs(kUsage, stdout);
            std::exit(0);
        } else if (a == "--version") {
            std::printf("foucault-sim %s\n", kVersion);
            std::exit(0);
        } else if (a == "--lat") o.params.latitude_deg = to_double("--lat", value());
        else if (a == "--length") o.params.length = to_double("--length", value());
        else if (a == "--gravity") o.params.gravity = to_double("--gravity", value());
        else if (a == "--amplitude") o.params.amplitude = to_double("--amplitude", value());
        else if (a == "--day") o.params.day = to_double("--day", value());
        else if (a == "--legacy-day") o.params.day = foucault::kLegacyDay;
        else if (a == "--omega-scale") o.params.omega_scale = to_double("--omega-scale", value());
        else if (a == "--model") {
            const std::string m = value();
            if (m == "simplified") o.params.model = foucault::Model::Simplified;
            else if (m == "full") o.params.model = foucault::Model::Full;
            else fail("unknown model '" + m + "' (use simplified or full)");
        } else if (a == "--dt") o.params.dt = to_double("--dt", value());
        else if (a == "--duration") o.duration = to_double("--duration", value());
        else if (a == "--hours") o.duration = 3600.0 * to_double("--hours", value());
        else if (a == "--stride") o.stride = to_long("--stride", value());
        else if (a == "--out") o.out = value();
        else if (a == "--precision") o.precision = static_cast<int>(to_long("--precision", value()));
        else if (a == "--velocities") o.velocities = true;
        else if (a == "--quiet") o.quiet = true;
        else fail("unknown option '" + a + "'");
    }
    if (!(o.duration > 0.0)) fail("duration must be positive");
    if (o.precision > 17) o.precision = 17;
    return o;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const Options opt = parse(argc, argv);
        const foucault::Pendulum pendulum(opt.params);
        const foucault::Params& p = pendulum.params();

        const long steps = std::lround(opt.duration / p.dt);

        std::FILE* file = stdout;
        if (opt.out != "-") {
            file = std::fopen(opt.out.c_str(), "w");
            if (!file) throw std::runtime_error("cannot open '" + opt.out + "' for writing");
        }

        std::fprintf(file, "# foucault-sim %s: lat=%g deg, L=%g m, g=%g m/s^2, a=%g m, day=%.4f s, "
                           "omega_scale=%g, model=%s, dt=%g s\n",
                     kVersion, p.latitude_deg, p.length, p.gravity, p.release(), p.day, p.omega_scale,
                     foucault::to_string(p.model), p.dt);
        std::fprintf(file, opt.velocities ? "# t[s]\tx[m]\ty[m]\tvx[m/s]\tvy[m/s]\n" : "# t[s]\tx[m]\ty[m]\n");

        foucault::State s = pendulum.initial();
        for (long step = 0; step <= steps; ++step) {
            if (step % opt.stride == 0) {
                const double t = static_cast<double>(step) * p.dt;
                if (opt.velocities) {
                    std::fprintf(file, "%.*g\t%.*g\t%.*g\t%.*g\t%.*g\n", opt.precision, t, opt.precision, s.x,
                                 opt.precision, s.y, opt.precision, s.vx, opt.precision, s.vy);
                } else {
                    std::fprintf(file, "%.*g\t%.*g\t%.*g\n", opt.precision, t, opt.precision, s.x,
                                 opt.precision, s.y);
                }
            }
            s = pendulum.step(s);
        }

        if (file != stdout) std::fclose(file);

        if (!opt.quiet) {
            const double hours = p.precession_period() / 3600.0;
            std::fprintf(stderr,
                         "model %s, latitude %.2f deg, %ld steps of %g s (%.1f s)\n"
                         "swing period      %.3f s\n"
                         "precession rate   %.4f deg/h (%s)\n"
                         "precession period %.3f h\n",
                         foucault::to_string(p.model), p.latitude_deg, steps, p.dt,
                         static_cast<double>(steps) * p.dt, p.swing_period(),
                         p.precession_rate() * (180.0 / foucault::kPi) * 3600.0,
                         p.precession_rate() >= 0.0 ? "clockwise from above" : "counter-clockwise from above",
                         hours);
        }
        return 0;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "foucault-sim: %s\n", e.what());
        return 1;
    }
}
