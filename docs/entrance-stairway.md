# Entrance breakable stair section

The first-visit and return Entrance now run the original sixteen-HP stair section
before Death’s room. Weapon damage opens the two collision tiles, retains the
permanent broken flag and drops the original turkey. Re-entry restores the open
tiles without creating a second reward.

Its selected death effect reserves the original sixteen primitive slots, spins and
drops a textured tile fragment, then creates six falling pieces and selected smoke.
The original exhausted-primitive fallback still completes the break and progression.
Foreground texture-page/UV/palette metadata is explicitly borrowed for the room;
room entry clears the views. The shared intense-explosion routine now uses the
correct private initializer for every supported stage profile.

Synthetic regressions cover ordinary damage, tile changes, the food factory,
fragment artwork/motion, ownership/cleanup, saved restoration, missing metadata,
primitive exhaustion and both stage identities. Private native LLDB checks exercise
weapon strikes, presentation, debris and cleanup in NO3/NP3 with explicit protected
fixtures. They do not establish continuous-route or console parity. Full raster,
priority, effects and timing parity remain incomplete.

Adapted from `st/e_stairway.h` and selected `st/e_misc.h` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under AGPL-3.0-or-later.
Tile artwork and game definitions remain in private runtime files.
