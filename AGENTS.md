# Working in sotn-native

- This repository currently contains an original SDL3 bootstrap, not a game port.
- Never add ROMs, BIOS images, extracted game data, original game code, or assets.
  End users must supply their own game dump. Local inputs belong outside this
  checkout or in ignored `local/`; CI and release packages must work without them.
- Use `src/main.cpp` as the single app entrypoint. Future reusable code belongs
  in focused modules. List sources explicitly in CMake; do not glob them.
- C++20, Google naming/design conventions, Microsoft clang-format layout.
  Warnings are errors for project sources, never for fetched dependencies.
- Pin dependencies with integrity checks. SDL3 is linked statically; the Android
  JNI app is a shared library containing that static SDL3 implementation.
- Use `make`, `make test`, `make package`; keep desktop, browser, Android, and
  iOS CI working. Browser frame pacing belongs to SDL's callback main loop.
- Run `python3 scripts/check_repository.py` before committing or publishing.
- Keep the bootstrap small. Do not start reverse engineering or asset extraction
  without an explicit task requesting that work.
