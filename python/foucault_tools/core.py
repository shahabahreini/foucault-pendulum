"""Pure analysis: parameters, the exact solution and the swing-plane angle."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

import numpy as np

SIDEREAL_DAY = 86164.0905  # s, rotation period of the Earth against the stars
LEGACY_DAY = 84600.0  # s, the typo in the 2021 course code


@dataclass(frozen=True)
class Params:
    """Mirror of the C++ ``foucault::Params`` (defaults match the course project)."""

    gravity: float = 9.83
    length: float = 67.0
    latitude_deg: float = 49.0
    amplitude: float | None = None  # defaults to length / 100
    day: float = SIDEREAL_DAY
    omega_scale: float = 1.0
    dt: float = 0.1
    model: str = "simplified"

    @property
    def release(self) -> float:
        return self.amplitude if self.amplitude else self.length / 100.0

    @property
    def omega(self) -> float:
        return self.omega_scale * 2.0 * np.pi / self.day

    @property
    def precession_rate(self) -> float:
        """Omega sin(latitude) in rad/s; positive is clockwise seen from above."""
        return self.omega * np.sin(np.radians(self.latitude_deg))

    @property
    def swing_period(self) -> float:
        return 2.0 * np.pi * np.sqrt(self.length / self.gravity)


def precession_period_hours(latitude_deg, day: float = SIDEREAL_DAY):
    """Time for the swing plane to turn once, ``day / |sin(latitude)|`` in hours."""
    s = np.abs(np.sin(np.radians(latitude_deg)))
    with np.errstate(divide="ignore"):
        return day / s / 3600.0


def exact_simplified(p: Params, t) -> np.ndarray:
    """Closed-form ``x + iy`` of the simplified model released from rest at ``x = a``.

    zeta(t) = a exp(-i s t) [cos(w t) + i (s / w) sin(w t)],
    with s = Omega sin(latitude) and w = sqrt(g / L + s^2).
    """
    t = np.asarray(t, dtype=float)
    s = p.precession_rate
    w = np.sqrt(p.gravity / p.length + s * s)
    return p.release * np.exp(-1j * s * t) * (np.cos(w * t) + 1j * (s / w) * np.sin(w * t))


def load_trace(path: str | Path) -> np.ndarray:
    """Read a ``foucault-sim`` file into an ``(n, 3)`` array of ``t, x, y``."""
    return np.loadtxt(path, comments="#", usecols=(0, 1, 2))


def turning_points(t: np.ndarray, x: np.ndarray, y: np.ndarray) -> np.ndarray:
    """Indices of the local maxima of the distance from the vertical."""
    r = np.hypot(x, y)
    inner = (r[1:-1] > r[:-2]) & (r[1:-1] >= r[2:])
    return np.flatnonzero(inner) + 1


def plane_angle_deg(t: np.ndarray, x: np.ndarray, y: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    """Angle of the swing line at each turning point, unwrapped, in degrees.

    The angle of a line is defined modulo 180 degrees; this unwraps it so the slope
    against time is the signed precession rate. Returns ``(times, angles)``.
    """
    idx = turning_points(t, x, y)
    angle = np.arctan2(y[idx], x[idx])
    # Successive turning points lie on opposite sides of the vertical: fold to a line.
    line = np.degrees(np.unwrap(2.0 * angle) / 2.0)
    # A line at 179.97 degrees is the line at -0.03: start within (-90, 90].
    line -= 180.0 * np.round(line[0] / 180.0)
    return t[idx], line


def measured_precession_period_hours(t: np.ndarray, x: np.ndarray, y: np.ndarray) -> float:
    """Period from a straight-line fit to the unwrapped plane angle; ``inf`` if it does not turn."""
    times, angles = plane_angle_deg(t, x, y)
    if len(times) < 3:
        return float("inf")
    slope = np.polyfit(times, angles, 1)[0]  # deg / s
    if abs(slope) < 1e-12:
        return float("inf")
    return 360.0 / abs(slope) / 3600.0
