"""Command line: ``foucault-plot all --out figures``."""

from __future__ import annotations

import argparse
from pathlib import Path

from . import plots


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="foucault-plot", description="Draw the README figures.")
    parser.add_argument("which", choices=[*plots.ALL, "all"], help="figure to draw")
    parser.add_argument("--bin", help="path to the foucault-sim executable")
    parser.add_argument("--out", default="figures", help="output directory (default: figures)")
    args = parser.parse_args(argv)

    names = list(plots.ALL) if args.which == "all" else [args.which]
    for name in names:
        path = plots.ALL[name](Path(args.out), binary=args.bin)
        print(path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
