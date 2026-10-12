"""Analysis and plotting for the foucault-pendulum C++ solver."""

from .core import (
    SIDEREAL_DAY,
    Params,
    exact_simplified,
    load_trace,
    plane_angle_deg,
    precession_period_hours,
)
from .simulate import find_binary, simulate

__all__ = [
    "SIDEREAL_DAY",
    "Params",
    "exact_simplified",
    "find_binary",
    "load_trace",
    "plane_angle_deg",
    "precession_period_hours",
    "simulate",
]
__version__ = "1.0.0"
