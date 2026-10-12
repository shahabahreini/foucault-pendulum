"""The figures shown in the repository README. Every plot is computed, nothing is hand-drawn."""

from __future__ import annotations

from dataclasses import replace
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402

from .core import (  # noqa: E402
    LEGACY_DAY,
    SIDEREAL_DAY,
    Params,
    exact_simplified,
    measured_precession_period_hours,
    plane_angle_deg,
    precession_period_hours,
)
from .simulate import simulate  # noqa: E402

INK = "#1c1915"
OXBLOOD = "#7a2420"
SLATE = "#2c3e57"
GILT = "#a88a4a"
MUTE = "#6b645a"

RC = {
    "figure.dpi": 100,
    "savefig.dpi": 200,
    "font.family": "sans-serif",
    "axes.edgecolor": MUTE,
    "axes.labelcolor": INK,
    "axes.titlesize": 11,
    "axes.titleweight": "bold",
    "axes.spines.top": False,
    "axes.spines.right": False,
    "xtick.color": MUTE,
    "ytick.color": MUTE,
    "grid.color": "#dcd6c8",
    "grid.linewidth": 0.6,
    "legend.frameon": False,
}


def _save(fig, out: Path, name: str) -> Path:
    out.mkdir(parents=True, exist_ok=True)
    path = out / name
    fig.savefig(path, bbox_inches="tight", facecolor="white")
    plt.close(fig)
    return path


def trace(out: Path, binary=None, latitude_deg: float = 49.0, hours: float = 4.0) -> Path:
    """Bob path seen from above for the first hours: the swing line fans out clockwise."""
    from matplotlib.collections import LineCollection
    from matplotlib.colors import LinearSegmentedColormap

    p = Params(latitude_deg=latitude_deg)
    t, x, y = simulate(p, hours * 3600.0, stride=5, binary=binary).T
    points = np.column_stack([x, y]).reshape(-1, 1, 2)
    segments = np.concatenate([points[:-1], points[1:]], axis=1)
    cmap = LinearSegmentedColormap.from_list("ink", [SLATE, OXBLOOD])
    with plt.rc_context(RC):
        fig, ax = plt.subplots(figsize=(5.4, 5.4))
        lines = LineCollection(segments, cmap=cmap, linewidths=0.35, alpha=0.55)
        lines.set_array(t[:-1] / 3600.0)
        ax.add_collection(lines)
        lim = p.release * 1.08
        ax.set(xlim=(-lim, lim), ylim=(-lim, lim), xlabel="x  (m)", ylabel="y  (m)")
        ax.set_aspect("equal")
        ax.set_title(f"Bob path, first {hours:g} h at latitude {latitude_deg:g}°", loc="left")
        ax.grid(True)
        cbar = fig.colorbar(lines, ax=ax, fraction=0.046, pad=0.03)
        cbar.set_label("time  (h)")
        turned = np.degrees(p.precession_rate) * hours * 3600.0
        ax.text(0.02, 0.02, f"swing plane turned {turned:.1f}° clockwise",
                transform=ax.transAxes, color=MUTE, fontsize=9)
    return _save(fig, out, "trace-first-hours.png")


def components(out: Path, binary=None) -> Path:
    """x(t) and y(t) for the first hour: Coriolis feeds the y motion from zero."""
    p = Params()
    data = simulate(p, 3600.0, binary=binary)
    t, x, y = data.T
    with plt.rc_context(RC):
        fig, (a, b) = plt.subplots(2, 1, figsize=(8, 4.6), sharex=True, constrained_layout=True)
        a.plot(t / 60, x, color=INK, lw=0.7)
        a.set(ylabel="x  (m)")
        a.set_title("Simplified model, latitude 49°, first hour", loc="left")
        b.plot(t / 60, y, color=OXBLOOD, lw=0.7)
        b.set(ylabel="y  (m)", xlabel="time  (min)")
        for ax in (a, b):
            ax.grid(True)
    return _save(fig, out, "components-first-hour.png")


def sweep(out: Path, binary=None) -> Path:
    """Measured precession period against latitude, with ``sidereal day / sin(latitude)``."""
    lats = np.array([10, 20, 30, 40, 49, 60, 70, 80, 90], dtype=float)
    measured = []
    for lat in lats:
        p = Params(latitude_deg=float(lat))
        duration = min(precession_period_hours(lat) * 3600.0, 12 * 3600.0)
        t, x, y = simulate(p, duration, stride=10, binary=binary).T
        measured.append(measured_precession_period_hours(t, x, y))
    measured = np.array(measured)
    grid = np.linspace(8, 90, 300)
    with plt.rc_context(RC):
        fig, (a, b) = plt.subplots(1, 2, figsize=(9.2, 3.9), constrained_layout=True,
                                   gridspec_kw={"width_ratios": [1.5, 1]})
        a.plot(grid, precession_period_hours(grid), color=INK, lw=1.2, label="sidereal day / sin(latitude)")
        a.plot(grid, precession_period_hours(grid, LEGACY_DAY), color=GILT, lw=1.0, ls="--",
               label="with the 2021 typo (84600 s)")
        a.plot(lats, measured, "o", color=OXBLOOD, ms=5, label="measured in the simulation")
        a.set(xlabel="latitude  (°)", ylabel="precession period  (h)", ylim=(0, 150), xlim=(0, 92))
        a.set_title("Swing-plane precession period", loc="left")
        a.legend(loc="upper right", fontsize=8)
        a.grid(True)
        rel = (measured / precession_period_hours(lats) - 1.0) * 1e6
        b.axhline(0, color=MUTE, lw=0.8)
        b.plot(lats, rel, "o-", color=OXBLOOD, ms=4, lw=0.8)
        b.set(xlabel="latitude  (°)", ylabel="measured / analytic − 1  (ppm)")
        b.set_title("Difference", loc="left")
        b.grid(True)
    return _save(fig, out, "precession-period-vs-latitude.png")


def convergence(out: Path, binary=None) -> Path:
    """Maximum error against the closed form for decreasing time steps; slope 4 means RK4."""
    p0 = Params()
    steps = [2.0, 1.0, 0.5, 0.25, 0.1, 0.05]
    errors = []
    for dt in steps:
        p = replace(p0, dt=dt)
        t, x, y = simulate(p, 600.0, binary=binary).T
        errors.append(np.max(np.abs(x + 1j * y - exact_simplified(p, t))))
    steps_a, errors_a = np.array(steps), np.array(errors)
    with plt.rc_context(RC):
        fig, ax = plt.subplots(figsize=(5.4, 4.0))
        ax.loglog(steps_a, errors_a, "o-", color=OXBLOOD, ms=5, lw=1, label="RK4 error")
        ref = errors_a[2] * (steps_a / steps_a[2]) ** 4
        ax.loglog(steps_a, ref, color=MUTE, ls="--", lw=0.9, label="slope 4")
        ax.set(xlabel="time step  dt  (s)", ylabel="max |numerical − exact|  (m)")
        ax.set_title("Convergence over 600 s", loc="left")
        ax.legend()
        ax.grid(True, which="both")
    return _save(fig, out, "rk4-convergence.png")


def models(out: Path, binary=None) -> Path:
    """Difference between the simplified and the full model at Earth's rate and at 300 times."""
    with plt.rc_context(RC):
        fig, ax = plt.subplots(figsize=(8, 3.6))
        for scale, colour in ((1.0, INK), (300.0, OXBLOOD)):
            a = simulate(Params(omega_scale=scale), 3500.0, binary=binary)
            b = simulate(Params(omega_scale=scale, model="full"), 3500.0, binary=binary)
            d = np.hypot(a[:, 1] - b[:, 1], a[:, 2] - b[:, 2])
            ax.semilogy(a[:, 0] / 60, np.maximum(d, 1e-12), color=colour, lw=0.8,
                        label=f"Ω × {scale:g}")
        ax.set(xlabel="time  (min)", ylabel="|simplified − full|  (m)")
        ax.set_title("Centrifugal terms: simplified against full model", loc="left")
        ax.legend()
        ax.grid(True, which="both")
    return _save(fig, out, "simplified-vs-full.png")


def plane_angle(out: Path, binary=None) -> Path:
    """Swing-plane angle at each turning point against time, with the analytic line."""
    p = Params()
    hours = 6
    t, x, y = simulate(p, hours * 3600.0, stride=10, binary=binary).T
    times, angle = plane_angle_deg(t, x, y)
    with plt.rc_context(RC):
        fig, ax = plt.subplots(figsize=(6.4, 3.8))
        ax.plot(times / 3600, angle, ".", color=OXBLOOD, ms=2, label="simulation (turning points)")
        ax.plot(times / 3600, -np.degrees(p.precession_rate) * times, color=INK, lw=0.9,
                label="−Ω sin(latitude) · t")
        ax.set(xlabel="time  (h)", ylabel="swing-plane angle  (°)")
        ax.set_title("Clockwise precession at latitude 49°", loc="left")
        ax.legend()
        ax.grid(True)
    return _save(fig, out, "plane-angle-vs-time.png")


ALL = {
    "trace": trace,
    "components": components,
    "sweep": sweep,
    "convergence": convergence,
    "models": models,
    "angle": plane_angle,
}
