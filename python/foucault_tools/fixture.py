"""Write the C++ solver's output for a few cases as JSON, to test other implementations against it.

The website's JavaScript port of the model reads this file (scripts/fixtures/foucault-cpp.json there).
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from .core import Params
from .simulate import simulate

CASES = [
    {"name": "49N", "latitude_deg": 49.0, "model": "simplified", "omega_scale": 1.0, "duration": 7200.0, "stride": 100},
    {"name": "30S", "latitude_deg": -30.0, "model": "simplified", "omega_scale": 1.0, "duration": 7200.0, "stride": 100},
    {"name": "pole", "latitude_deg": 90.0, "model": "simplified", "omega_scale": 1.0, "duration": 7200.0, "stride": 100},
    {"name": "equator", "latitude_deg": 0.0, "model": "simplified", "omega_scale": 1.0, "duration": 3600.0, "stride": 100},
    {"name": "49N-x300-full", "latitude_deg": 49.0, "model": "full", "omega_scale": 300.0, "duration": 3600.0, "stride": 50},
    {"name": "49N-x300-simplified", "latitude_deg": 49.0, "model": "simplified", "omega_scale": 300.0, "duration": 3600.0, "stride": 50},
]


def build(binary=None) -> dict:
    out = {"solver": "foucault-sim", "dt": 0.1, "gravity": 9.83, "length": 67.0, "amplitude": 0.67, "cases": []}
    for case in CASES:
        p = Params(latitude_deg=case["latitude_deg"], model=case["model"], omega_scale=case["omega_scale"], dt=0.1)
        data = simulate(p, case["duration"], stride=case["stride"], binary=binary)
        out["cases"].append({**case, "t": data[:, 0].tolist(), "x": data[:, 1].tolist(), "y": data[:, 2].tolist()})
    return out


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bin", help="path to foucault-sim")
    parser.add_argument("--out", required=True, help="JSON file to write")
    args = parser.parse_args(argv)
    Path(args.out).write_text(json.dumps(build(args.bin), separators=(",", ":")))
    print(args.out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
