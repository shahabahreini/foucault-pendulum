# Method

This note derives the equations that `foucault-sim` integrates, gives the closed-form solution the tests
compare against, and describes the numerical method and its limits.

## 1. Setting

Use a frame fixed to the Earth at latitude λ with axes **x** north, **y** west and **z** up (a right-handed set).
The Earth turns at

    Ω = 2π / T_sid,    T_sid = 86164.0905 s,

so Ω = 7.2921 × 10⁻⁵ rad/s. In these axes Ω = Ω (cos λ, 0, sin λ).

The pendulum has length L and a bob of mass m. For a bob that stays near the vertical, z stays within
x²/2L of zero, and the tension is T ≈ mg along the wire.

## 2. Forces in the rotating frame

Newton's second law in a rotating frame has two extra terms:

    r̈ = g + T/m − 2 Ω × ṙ − Ω × (Ω × r)

**Restoring force.** The horizontal part of the tension is −mg·(x/L, y/L), so each horizontal axis gets the
acceleration −(g/L) x. The mass cancels.

**Coriolis force.** With v = (ẋ, ẏ, 0),

    −2 Ω × v = 2 Ω ( sin λ · ẏ,  −sin λ · ẋ,  cos λ · ẋ )

so the horizontal components are

    a_x = +2 Ω sin λ · ẏ
    a_y = −2 Ω sin λ · ẋ

Only the vertical part of Ω, Ω sin λ, bends horizontal motion. That is why the effect vanishes at the equator,
is strongest at the poles and changes sign between hemispheres. The vertical component, 2Ω cos λ · ẋ, is about
10⁻⁵ of g and is dropped.

## 3. The equations that are solved

Collecting the terms gives the **simplified model**:

    ẍ − 2 Ω sin λ ẏ + (g/L) x = 0
    ÿ + 2 Ω sin λ ẋ + (g/L) y = 0

With Ω × (Ω × r) evaluated for the axes above (and z ≈ 0), the centrifugal term adds Ω² sin²λ · x to the
x acceleration and Ω² · y to the y acceleration. This is the **full model**:

    ẍ − 2 Ω sin λ ẏ + (g/L − Ω² sin²λ) x = 0
    ÿ + 2 Ω sin λ ẋ + (g/L − Ω²)         y = 0

The two coefficients differ because x points north and y points west: the part of Ω perpendicular to a
horizontal axis differs between the two. This is the same pair of equations as in the 2021 report (its
equations 26 and 27), and the code in this repository selects between them with `--model simplified|full`.

## 4. Closed form and the precession period

Put ζ = x + i y. The simplified pair becomes one complex equation,

    ζ̈ + 2 i Ω sin λ ζ̇ + (g/L) ζ = 0.

Its solution for release from rest at x = a is

    ζ(t) = a · exp(−i Ω sin λ · t) · [ cos(w t) + i (Ω sin λ / w) sin(w t) ],    w = √(g/L + (Ω sin λ)²).

The bracket is the swing, with frequency w ≈ √(g/L) (period 2π√(L/g) = 16.40 s for L = 67 m). The factor in front
is a rotation of the whole picture at the rate Ω sin λ. Seen from above the swing plane therefore turns by

    Δθ = −Ω sin λ · t     (clockwise for λ > 0),

and returns to its starting direction after

    T_prec = 2π / (Ω sin λ) = T_sid / sin λ.

| latitude | T_prec | rate |
|---|---|---|
| 90° (pole) | 23.93 h | 15.04 °/h |
| 49° | 31.71 h | 11.35 °/h |
| 30° | 47.87 h | 7.52 °/h |
| 0° (equator) | no rotation | 0 |

The swing is a line, so it looks the same after half of this time; the figure quoted in the literature,
`T_sid / sin λ`, is the full turn of the plane.

## 5. Size of the centrifugal terms

At Earth's rate Ω² ≈ 5.3 × 10⁻⁹ s⁻² against g/L ≈ 0.147 s⁻², a ratio of 3.6 × 10⁻⁸. The full and simplified models
therefore differ by about 10⁻⁶ m after an hour (see `figures/simplified-vs-full.png`). With Ω multiplied by 300,
as in the course report, the ratio is 3 × 10⁻³ and the models separate within minutes.

## 6. Numerical method

The state is (x, y, ẋ, ẏ). `foucault::rk4_step` applies the classical fourth-order Runge-Kutta scheme

    y_{n+1} = y_n + (dt / 6)(k₁ + 2k₂ + 2k₃ + k₄)

to the first-order form of the equations. With dt = 0.1 s there are about 164 steps per swing.

The tests check three things:

1. **Agreement with the closed form.** Over one hour at dt = 0.1 s the largest difference is below 0.05 mm. The
   remaining error is a slow drift in the swing phase of about 17 µm per hour; it does not affect the
   precession.
2. **Order of convergence.** Halving dt reduces the error by about 16, as expected of a fourth-order scheme
   (`figures/rk4-convergence.png`).
3. **Precession angle.** The angle of the swing line at the last turning point agrees with −Ω sin λ · t to better
   than 0.05° at latitudes 10°, 49° and 90°, and the sign flips in the southern hemisphere.

## 7. What is not modelled

- Air drag and wire stiffness, which make real pendulums lose amplitude.
- Starts that are not from rest, which add an elliptical motion and an extra drift of the swing plane
  (Airy precession).
- The large-angle pendulum. The solver rejects release distances above 20% of the length.
- The vertical component of the Coriolis force and the variation of g with height.

## References

See [references.md](references.md).
