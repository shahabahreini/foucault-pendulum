# Notes on the 2021 course report

The original report ("Course Project", UAlbany Computational Physics, May 2021) is not distributed here because
it contains figures from other sources. This page records what changed between it and this repository.

## Changes to the results

- **Earth's rotation period.** The report's Table 1 lists 86400 s and its text gives Ω ≈ 7.2722 × 10⁻⁵ rad/s
  (which is 2π / 86400), but the code used `2π / 84600`, a typo. This repository uses the sidereal day, 86164.0905 s.
  The precession period at 49° is 31.71 h; the 2021 code gave 31.14 h. `foucault-sim --legacy-day` reproduces the
  old numbers and the test suite checks that it does.
- **Simulated time.** The report's Figure 5 caption says 9999.9 s. The code that produced the shipped output files
  ran 35000 steps of 0.1 s, which is 3499.9 s, about 3% of one precession period. The new examples run for
  hours, so the full turn of the swing plane is visible.

## Corrections to the text

- **Equation 7** reads `θ̈ = mg sin θ`. The pendulum equation is `θ̈ = −(g/l) sin θ`: the mass cancels and the
  sign is restoring.
- **Equation references.** The text calls the two models "equation (25) and (26)"; they are equations (24)–(25)
  (simplified) and (26)–(27) (full).

## What the 2021 code did right

- The hand-written RK4 stages are the classical scheme for the coupled system. The new code gives the same
  numbers: `ctest` compares the first hour against the original files to six significant digits.
- The asymmetry between the two centrifugal terms in the full model, `Ω² sin²λ` on x and `Ω²` on y, is correct for
  axes x north and y west. See `method.md`, section 3.

## Code changes

Hard-coded constants and a fixed output file name are now command-line options, global state is a `Pendulum`
class, the four unrolled RK4 stages are one templated function, output carries a header and 12 significant
digits, and the live-animation helper (`usleep`, a re-read of the whole file for every frame) is replaced by
`foucault-plot`.
