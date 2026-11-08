# Selected holy water

Holy water (subweapon 3) has moved from the bounded [subweapon service](laboratory-subweapons.md)
to the shared player runtime, after the [dagger](player-dagger.md) and [axe](player-axe.md). Hold
Up and tap either attack button. Defaults, grants and saves are unchanged.

## Use

It uses the shared `func_8010EB5C` path ([Vibhuti](player-vibhuti.md#use)) with
`g_SubwpnDefs[3]`:

- **Cost.** Three hearts (Heart broach applies).
- **Chain.** Three; only bottles count, not their glass or flames.
- **Factory.** Blueprint 27 (entity 22, slots 32..47, origin 2).
- **Animation.** Animation 0xBC and the shared casting grunt.

## Bottle

`EntitySubwpnHolyWater` is an entity sprite (DRA bank 9, frame 0x1D) with no primitive rows.

- **Birth.** Flags `FLAG_POS_CAMERA_LOCKED`; priority two below the player's.
  - It drops eight pixels and flies at 1.25 pixels an update (`SetSpeedX`), lifting at 3.125.
  - `func_8011A290` rolls its attack, takes the least-used group 7..10 and records crash id 11.
  - Its hitbox is 4x4; timer 10 is set.
- **Flight.** Gravity adds 28/128 per update while the bottle is slower than four pixels an
  update. `CheckHolyWaterCollision` lands it on the floor's first solid pixel.
  - It also snaps it onto sloped surfaces and lets a side-only tile above the floor pass.
  - After the horizontal move, `func_80125B6C` probes four pixels ahead and seven up. A side
    wall snaps it to that side with the pixel fraction cleared.
- **Break.** On landing or a wall:
  - It plays HOLY_WATER_ATTACK and requests `FACTORY(59)` (the glass), then hides its sprite.
  - Its hitbox stays live. On the next four updates whose timer is a multiple of four it requests
    `FACTORY(28 | D_8013841C << 16, 11 << 9)`, a flame, and counts `D_8013841C` up.
  - It retires four updates later.

`D_8013841C` is a DRA BSS counter that only sets the flames' unused parameter byte. The runtime
keeps it as `holy_counter` (never saved).

## Glass

`EntitySubwpnHolyWaterBreakGlass` (entity 40, slots 32..63, subweapon id zero) takes 16 rows.

- **Carriers.** Eight hidden FakePrim carriers start at the bottle's whole pixel. Each rolls a
  spread of ±0.25..0.5 pixels and a lift of 2.5 to 3.0 pixels.
- **Shards.** Eight G4 shards roll their blue/green colours and an additive or opaque draw mode.
  That is 48 rand calls in all.
- **Fall.** The carriers fall at 36/128 per update, with the shards placed around them from
  `D_800B0658`. The glass retires after twenty updates.

## Flames

`EntitySubwpnHolyWaterFlame` (entity 23, subweapon 11):

- **Birth.**
  - It rolls its colour before `AllocPrimitives(GT4, 4)`, so a failed allocation still spends
    three rand calls.
  - It then rolls subweapon 11's attack, requests `FACTORY(4, 7)` ten pixels above it, and rolls
    a height of 0x12..0x21.
- **Smoke.** That factory starts green profile-7 smoke: eight tiles with player-particle
  behaviour 3, like profiles 4 and 11.
- **Update.** Four additive strips (page 0x1A, palette 0x1B2) wave about five points
  (`rsin >> 10`).
  - Their height follows `rsin((timer - 0x10) * 64 + 0xC00) >> 8`, and they hide below seven
    pixels.
  - UVs come from `D_800B0688` by `g_GameTimer & 7`.
  - The hitbox is live one update in four; the colour fades below 0x29.
  - The flame retires at 0x10.

All three entities keep their place in the room (`FLAG_POS_CAMERA_LOCKED` and `DRAW_UNK02`).
Without `KEEP_ALIVE_OFFCAMERA`, the runtime retires them off screen. Room entry retires them and
their factories.

## Validation

- **Public tests.**
  - Bottle: birth and facing, gravity, the floor landing with no row allocation, the side-only
    wall snap, the side-only tile above the floor, the break, the four counted flames and
    retirement.
  - Glass: the exact rolls and the fall.
  - Flames: the colour rolls before allocation, the smoke factory, and the exact
    wave/height/hitbox/fade.
  - Profile-7 smoke birth and lifetime.
  - The runtime's rowless bottle, glass, flames and smoke.
  - Mutation: 34 of 43 mutants fail the tests and the oracle catches seven more; two are
    equivalent (a kind mask, and a speed cap the bottle never reaches exactly).
- **Private emulator comparison.**
  - It runs the original `EntitySubwpnHolyWater` (with the real `CheckHolyWaterCollision` and
    `func_80125B6C`), `EntitySubwpnHolyWaterBreakGlass` and `EntitySubwpnHolyWaterFlame` on the
    actual DRA/SLUS.
  - The map holds blocks, side-only, sideless and sloped tiles.
  - It covers 1,050 contexts and 46,764 callbacks: 750 bottles and 3,000 flame factories with
    the counter.
- **Private LLDB probe.** It uses holy water in the Laboratory by pad input, facing both ways.
  - The bottle lands, and its glass, flames and smoke all draw.
  - A held zombie takes repeated flame hits.

These use explicit subweapon/hearts/protection/position/enemy-HP fixtures, not acquisition,
route or console evidence.

Adapted source: `src/dra/84B88.c` and `7E4BC.c` at the pinned reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
