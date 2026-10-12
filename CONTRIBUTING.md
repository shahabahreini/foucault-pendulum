# Contributing

Bug reports, questions and pull requests are welcome.

```bash
cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure
cd python && pip install -e ".[test]" && pytest
```

- Keep the C++ core header-only and free of third-party dependencies.
- Add a test for every behaviour change. The physics tests compare against the closed-form solution, so a
  new feature should be checkable the same way where possible.
- Describe physics changes with a reference or a derivation in `docs/method.md`.
