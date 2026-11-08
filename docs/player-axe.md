# Selected axe

The axe (subweapon 2) has moved from the bounded [subweapon service](laboratory-subweapons.md) to
the shared player runtime, after the [dagger](player-dagger.md). Hold Up and tap either attack
button. Defaults, grants and saves are unchanged.

## Use

It uses the shared `func_8010EB5C` path ([Vibhuti](player-vibhuti.md#use)) with
`g_SubwpnDefs[2]`:

- **Cost.** Four hearts (Heart broach applies).
- **Chain.** Two axes at once.
- **Factory.** Blueprint 9: entity 10 in slots 32..47, origin 2 (the player's position at its
  first update).
- **Animation.** Standing animation 0x5D and the shared casting grunt.

## Axe

`EntitySubwpnThrownAxe` takes five GT4 rows: the axe and four afterimages.

- **Rows.** Page 0x1C, one 24x40 image. The CLUT alternates 0x1AB/0x1AC with `g_GameTimer >> 1`.
  - All five rows sit two below the player's priority.
  - No row has `DRAW_UNK02`, so their screen words do not follow the camera. Row 0 is recomputed
    from the entity each update.
- **Flags.** `FLAG_POS_CAMERA_LOCKED | KEEP_ALIVE_OFFCAMERA | HAS_PRIMS | FLAG_UNK_20000`. The axe
  keeps flying through a room change; its position takes the room shift.
- **Birth.**
  - Its facing is the reverse of the player's. `SetSpeedX(FIX(-2))` still sends it forward.
  - It launches at 6 pixels an update upward and rises twelve pixels unless the player crouches.
  - `func_8011A290` rolls its attack and picks the least-used group 7..10.
  - It has a 12x12 hitbox, plays SWISH_C and sets timer 10.
- **Flight.**
  - It spins 0x80 per update.
  - Every quarter turn it swishes through `PlaySfxVolPan` at a volume starting at 0x7F and
    falling by eight. The zero clamp is unreachable before it leaves the screen.
  - It falls by 34/128 pixel per update, capped at eight, and retires below screen line 0x100.
  - Ordinary hits pass through. A hitFlags 2 contact (a guard) clears the hitbox and reverses at
    half speed (truncated), lifting at three pixels. The spin then turns the other way at 0xC0,
    with gravity 18/128.
- **Afterimages.** Every third update the next afterimage restarts.
  - On that update it copies the axe row's words.
  - It is then tinted additively from `D_800B0628` for ten updates and hidden.
  - The axe's corners are `(rcos/rsin << 4) * 21 >> 16` around its position.

## Validation

- **Public tests.**
  - Birth: facing, lift and crouch.
  - Exact flight: spin, swish volume, gravity, the pose cycle, afterimage copies/tints and the
    corners.
  - The volume clamp, both spin directions, and the retirement boundary.
  - The hitFlags 2 bounce, with truncated halving; hitFlags 1 and 3 pass through.
  - The runtime's rows, volume swish and room retention.
  - Mutation: 29 of 31 mutants fail the tests. The other two, one-unit corner angles, are
    caught by the oracle.
- **Private emulator comparison.** The original `EntitySubwpnThrownAxe` runs on the actual
  DRA/SLUS, with the real `func_8011A290`, `SetSpeedX` and `rcos`/`rsin`.
  - It covers 400 contexts and 25,959 callbacks: 62 bounces, 2,406 volume swishes and 80,777
    afterimage rows.
  - It catches deliberate corner mutations.
- **Private LLDB probe.** The axe is used in the Laboratory by pad input, facing both ways.
  - It rises 65 pixels and lives 59 updates, swishing and drawing afterimages.
  - It damages a held zombie and refuses a third live axe.

These use explicit subweapon/hearts/protection/position/enemy-HP fixtures, not acquisition,
route or console evidence.

Adapted source: `src/dra/84B88.c` at the pinned reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
