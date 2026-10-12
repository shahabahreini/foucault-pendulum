# foucault-tools

Python helpers for the [foucault-pendulum](https://github.com/shahabahreini/foucault-pendulum)
C++ solver: load traces, compute the swing-plane angle, compare with the exact solution,
and draw the figures used in the repository README.

```bash
pip install -e ".[test]"
foucault-plot all --bin ../build/foucault-sim --out ../figures
```
