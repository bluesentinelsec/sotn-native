# Selected Rebound Stone

Rebound Stone (subweapon 7) now runs through the shared player runtime, after
[Vibhuti](player-vibhuti.md). Hold Up and tap either attack button. Defaults, grants and saves
are unchanged.

## Use

The shared `func_8010EB5C` path ([Vibhuti](player-vibhuti.md#use)) with `g_SubwpnDefs[7]`:
2 hearts, chain 3, blueprint 24 (`E_SUBWPN_REBOUND_STONE`, entity 20, slots 32..47, origin 2:
the factory takes the player's position when it runs) and standing animation 0x5D.

## Entity

`EntitySubwpnReboundStone` and `rebound_stone.h`:

- **Birth.** `AllocPrimitives(PRIM_LINE_G2, 16)` takes the first free rows (a failure destroys
  the entity before any RNG). Row 0 is visible, the rest hidden; all are grey 0xFF, priority
  player + 2, drawMode 0x33 (additive) and start a 20-update timer at the stone. The entity keeps
  `KEEP_ALIVE_OFFCAMERA`. The angle is 0xE80 (0x980 when the *player* faces left) plus
  `(rand() & 0x7F) - 0x40`, rolled **before** `func_8011A290`'s attack roll and group (7..10).
  A 4x4 hitbox, timer 10 = 4, then a CheckCollision at the stone: born inside solid ground, the
  stone holds its collision loop for four updates (it still moves). SWISH_C.
- **Flight.** Unless held, six steps of 16x the unit vector probe vertically then horizontally.
  Solid probes reflect the angle off the original table of flat and sloped floors, ceilings and
  walls (the first helper always reflects, the second only before this update's bounce); the
  first bounce of an update advances the trail index and stops the probing. A bounce requests
  `CreateEntFactoryFromEntity(self, 10, 0)` at the pre-move position (three bursts of eight
  profile-0 tink sparks) and SUBWEAPON_TINK. The stone then moves 4 pixels along its angle,
  unless it is more than 64 pixels outside the screen or has bounced fifteen times, which starts
  the fade-out.
- **Fade-out.** 0x40 updates; the hitbox state is cleared at 0x20; every trail row's timer is
  zeroed.
- **Trail.** A bounce opens trail row `bounces` at the pre-move position; the current row's end
  follows the stone. Each visible row waits out its timer, then fades its grey by 2 (4 during the
  fade-out), never below the step.

Profile-0 tink sparks (`func_8011E4BC` behavior 0) roll `rand()` as both an angle and a ×4/×8
speed through `rcos`/`rsin`, drift without gravity and fade by six per update; the burst ends
once red drops below eight. Native particle children still reserve their rows first-free rather
than through `func_800EDB58`'s contiguous run; this affects which rows they lease, not their
behavior.

## Validation

Public tests cover the birth roll order, the embedded hold, exact flight steps, fifteen
reflections in a closed box with a tink per bounce, the fade-out, the hitbox clear, first-free
rows, the runtime group/power and the tink sparks' birth. A private emulator comparison runs the
original `EntitySubwpnReboundStone` (with the real `func_8011A290`) on the actual DRA/SLUS
against the native callback over 400 contexts with nonzero scroll and sloped tiles; a mutation
of the slope table is caught. A private LLDB probe throws the stone in the Laboratory by pad input
both ways. These use explicit subweapon/hearts/protection/position fixtures, not acquisition,
route or console evidence.

Adapted source: `src/dra/84B88.c`, `rebound_stone.h`, `7E4BC.c` and `4DA70.c` at the pinned
reconstructed revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
