# Selected Cross

The Cross (subweapon 4) now runs through the shared player runtime, after
[Vibhuti](player-vibhuti.md), [Rebound Stone](player-rebound-stone.md), [Agunea](player-agunea.md)
and the [Bible](player-bible.md). Hold Up and tap either attack button. Defaults, grants and saves
are unchanged.

## Use

This uses the shared `func_8010EB5C` path ([Vibhuti](player-vibhuti.md#use)) with `g_SubwpnDefs[4]`:

- 100 hearts.
- Chain 1: only the beam (subweapon id 4) counts. Giant crosses and particles do not, so a new
  cross can be used while the last crosses still rise.
- Blueprint 6 (entity 8, slots 32..47, origin 1). Origin 1 means the factory starts at the
  player's screen position and keeps it.
- Standing animation 0xDC (0xDD at a ledge).

Staurolite doubles or triples both cross attacks ([subweapon gear](player-subweapon-gear.md)).

## Beam

`EntitySubwpnCrashCross` first calls `func_8010DFF0(1, 1)` every update, even when its row
allocation fails. That call resets the afterimages and holds timer 15 at four.

- **Birth.** One GT4 row.
  - Flags `FLAG_HAS_PRIMS` only.
  - Screen line 120, priority 0xC2.
  - `func_8011A290` rolls the attack, takes the least-used group 7..10 and records crash id 12.
  - The gradient row is uploaded with `LoadImage(D_800B0788)` (Hellfire uploads two rows; the
    cross uploads only the first).
  - Sounds: CRASH_CROSS, then TELEPORT_BANG_B. Timer 12 is set to four.
  - The row samples the gradient as a 15-bit direct page (0x11C), additive.
- **Rise.** It grows three pixels up and down each update, until its half height's low byte
  reaches 0x70 (38 updates). It then requests `FACTORY(7, 12 << 9)` and `FACTORY(8, 0)`.
- **Widening.** It grows one pixel each way on odd `g_Timer` frames, reaching 45 pixels.
- **Hold.** It holds for 0x80 updates. The last one clamps its edges to the screen (0..0xFF)
  and plays WEAPON_APPEAR.
- **Spread.** Each update both edges move out by `3 * (|x - 128| + 128) / 112` pixels.
  - Its grey rises by four up to 0xF8, modulating the gradient (`DRAW_COLORS`).
  - It retires once both edges pass (-0x80, 0x180).
- **Hitbox.** It spans the beam: the full height, and the half width, or the edges once spreading.
  The width is a byte, so it wraps on the widest spread, as in the original.

## Giant crosses

Blueprint 7's factory releases six `EntityGiantSpinningCross` (entity 7, subweapon id 12),
one every 24 updates.

- **Rows.** Each takes 46 contiguous GT4 rows (`func_800EDB58`). The rows use page 0x1C, with
  the private UVs and palettes 0x1A0.. from `D_800B0F94`.
- **Birth.** It rolls subweapon 12's attack and takes a 12x80 hitbox. It starts at screen line
  0x160, rising six pixels each update and plays FIREBALL_SHOT_B.
- **Rise.** Above the screen top it accelerates by half a pixel each update, and it retires
  past -0x40.
- **Model.** Each update the model orbits by `rcos`/`rsin` of an angle that advances 0x60, and
  spins about Y (`RotMatrix(0, spin, 0x40)`).
  - Projection uses `SetGeomOffset(x, 120)` and `SetGeomScreen(320)`.
  - Each row projects vectors 0, 1 and 3 (`RTPT`), culls back faces with `NCLIP < 0`, then
    projects vector 2. Culled rows are hidden and keep their last words.
- **Hitbox and priority.** The projected centre places the hitbox and its depth (`SZ >> 2`)
  gives every face's priority: `0xD0 - (z - 0x50)`, with the 0x1B6/0x10 extremes.

## Particles

Blueprint 8 starts `EntitySubwpnCrashCrossParticles` (entity 9, subweapon id zero), which lives
for 192 updates with 64 GT4 rows (page 0x1A, palette 0x1B0).

- **Activation.** Every fourth update above eight, it activates the first idle row.
- **Birth.** A new spark makes three libc rand calls. These set its column (9..245), its start
  line (0xD0 or 0xF0), its priority (the player's, -32..+31) and its rise (4..19 pixels).
- **Life.** `func_80119E78` places a 12-pixel quad, which shrinks to 8 pixels from the fourth
  frame. The quad advances one UV frame every second update. A spark hides at frame six or
  below line 24.

## Screen anchoring

None of the three entities has `FLAG_POS_CAMERA_LOCKED`, and no row has `DRAW_UNK02`, so they
keep their screen position as the camera moves.

- Their rows keep original screen words; only the entity positions follow the camera.
- Factories also lack that flag before their first update. Blueprint 7 never gains it
  (origin 1).
  - A subweapon's factory request now keeps its requester's screen position.
  - An initialized factory without the flag follows the camera after each update. In
    practice this is blueprint 7, which lives 120 updates, so every giant cross appears at the
    beam's screen column.
- Room entry retires all three entities and their pending factories.

## Validation

Public tests cover:

- **Beam.** The afterimage reset before a failed allocation, the birth, rise, factories,
  odd-frame widening, the hold and its clamps, the spread steps and hitbox, and retirement.
- **Giant cross.** The rise and acceleration, orbit and spin, the projected hitbox and priority,
  face order, `NCLIP` culling with retained words, retirement, and the private face/UV readers.
- **Particles.** Their exact rand calls, rise, frames, hide boundary and 45 activations.
- **Runtime.** The beam, six giant crosses with contiguous rows at the beam's column while the
  camera scrolls, chain counting and room cleanup.

A 38-mutant sweep (32 first-pass plus six re-runs after added tests) leaves no survivors.

A private emulator comparison runs the original `EntitySubwpnCrashCross`,
`EntityGiantSpinningCross` and `EntitySubwpnCrashCrossParticles` on the actual DRA/SLUS:

- It uses the real `func_8011A290`, `func_80119E78`, `rcos`/`rsin` and SDK matrix calls.
- Inline GTE operations run on a private PSY-Z register model.
- It covers 330 contexts and 56,627 callbacks with nonzero scroll and 158,550 projected faces,
  and catches a deliberate mutation.

Debug and ASan traces are identical. A private LLDB probe uses the Cross in the Laboratory by
pad input, facing both ways:

- The beam lives 285 updates.
- Six giant crosses appear at its screen column while Alucard walks and the camera scrolls 103
  pixels.
- The particles run, all three draw, and a held zombie takes 15 hits.

These use explicit subweapon/hearts/protection/position/enemy-HP fixtures, not acquisition,
route or console evidence.

Adapted source: `src/dra/84B88.c`, `wolfForm.c`, `7879C.c` and `4DA70.c` at the pinned
reconstructed revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
