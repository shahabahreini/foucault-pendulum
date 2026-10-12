# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [1.0.0] - 2026-10-12

First public release, a cleaned-up version of the 2021 UAlbany Computational Physics course project.

### Added
- Header-only C++17 core (`include/foucault/`): parameters with validation, the model, and a generic RK4 step.
- `foucault-sim` command line tool with every physical and numerical parameter as an option.
- Exact closed-form solution of the simplified model, used by the tests and plots.
- Tests (CTest): agreement with the closed form, fourth-order convergence, precession period at three
  latitudes, northern and southern sense, equator, simplified against full model, input validation, and
  reproduction of the 2021 output files.
- `foucault-tools` Python package and `foucault-plot` command that draw the README figures.
- Documentation of the method, the report errata and the references.

### Changed
- **Earth's rotation period is now the sidereal day, 86164.0905 s.** The 2021 code used 84600 s, a typo for
  86400 s, which made Omega about 1.8% too large and the precession period about 1.8% too short at every latitude.
  `--legacy-day` reproduces the old value.
- The RK4 step is one templated function instead of four hand-unrolled stages.
- Output is buffered, carries 12 significant digits and has a header, instead of six digits and a flush per step.
- The simplified and full models are a run-time option instead of a `#define`.
