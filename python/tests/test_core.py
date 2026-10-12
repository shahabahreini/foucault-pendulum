import numpy as np
import pytest

from foucault_tools import Params, exact_simplified, plane_angle_deg, precession_period_hours
from foucault_tools.core import SIDEREAL_DAY, measured_precession_period_hours


def test_period_at_poles_and_49_degrees():
    assert precession_period_hours(90.0) == pytest.approx(SIDEREAL_DAY / 3600.0)
    assert precession_period_hours(49.0) == pytest.approx(SIDEREAL_DAY / np.sin(np.radians(49.0)) / 3600.0)
    assert precession_period_hours(0.0) == np.inf


def test_exact_solution_starts_at_release_point():
    p = Params()
    z = exact_simplified(p, np.array([0.0]))
    assert z[0] == pytest.approx(p.release)


def test_exact_solution_plane_angle_follows_the_formula():
    p = Params()
    t = np.arange(0, 6 * 3600.0, 0.1)
    z = exact_simplified(p, t)
    times, angle = plane_angle_deg(t, z.real, z.imag)
    expected = -np.degrees(p.precession_rate) * times
    assert np.max(np.abs(angle - expected)) < 0.02
    period = measured_precession_period_hours(t, z.real, z.imag)
    assert period == pytest.approx(precession_period_hours(49.0), rel=1e-3)


def test_southern_hemisphere_turns_the_other_way():
    p = Params(latitude_deg=-49.0)
    t = np.arange(0, 3600.0, 0.1)
    z = exact_simplified(p, t)
    _, angle = plane_angle_deg(t, z.real, z.imag)
    assert angle[-1] > 0.0
