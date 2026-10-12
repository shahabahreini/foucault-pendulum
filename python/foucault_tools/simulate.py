"""Run the C++ solver and read its trace."""

from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path

import numpy as np

from .core import Params


def find_binary(explicit: str | os.PathLike | None = None) -> Path:
    """Locate ``foucault-sim``: argument, ``$FOUCAULT_SIM``, ``build/``, then ``$PATH``."""
    candidates: list[Path] = []
    if explicit:
        candidates.append(Path(explicit))
    if os.environ.get("FOUCAULT_SIM"):
        candidates.append(Path(os.environ["FOUCAULT_SIM"]))
    root = Path(__file__).resolve().parents[2]
    candidates += [root / "build" / "foucault-sim", root / "build" / "Release" / "foucault-sim.exe"]
    found = shutil.which("foucault-sim")
    if found:
        candidates.append(Path(found))
    for c in candidates:
        if c.is_file():
            return c
    raise FileNotFoundError(
        "foucault-sim not found. Build it with `cmake -S . -B build && cmake --build build`, "
        "or pass --bin / set FOUCAULT_SIM."
    )


def simulate(
    p: Params,
    duration: float,
    *,
    stride: int = 1,
    binary: str | os.PathLike | None = None,
    legacy_day: bool = False,
) -> np.ndarray:
    """Return an ``(n, 3)`` array of ``t, x, y`` from the C++ solver."""
    cmd = [
        str(find_binary(binary)),
        "--quiet",
        "--lat", repr(float(p.latitude_deg)),
        "--length", repr(float(p.length)),
        "--gravity", repr(float(p.gravity)),
        "--amplitude", repr(float(p.release)),
        "--omega-scale", repr(float(p.omega_scale)),
        "--model", p.model,
        "--dt", repr(float(p.dt)),
        "--duration", repr(float(duration)),
        "--stride", str(stride),
        "--precision", "15",
    ]
    cmd += ["--legacy-day"] if legacy_day else ["--day", repr(float(p.day))]
    out = subprocess.run(cmd, check=True, capture_output=True, text=True).stdout
    rows = [line.split("\t") for line in out.splitlines() if line and not line.startswith("#")]
    return np.array(rows, dtype=float)
