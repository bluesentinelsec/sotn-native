# Selected dagger

The dagger (subweapon 1) has moved from the bounded [subweapon service](laboratory-subweapons.md)
to the shared player runtime, next to [Vibhuti](player-vibhuti.md), [Rebound
Stone](player-rebound-stone.md), [Agunea](player-agunea.md), the [Bible](player-bible.md) and the
[Cross](player-cross.md). Hold Up and tap either attack button. Defaults, grants and saves are
unchanged.

## Use

The shared `func_8010EB5C` path ([Vibhuti](player-vibhuti.md#use)) with `g_SubwpnDefs[1]`:

- one heart (Heart broach applies);
- chain 3;
- blueprint 3 (entity 4, slots 32..47, origin 2: the player's position at its first update);
- standing animation 0x5D;
- the shared casting grunt.

## Blade

`EntitySubwpnKnife` takes two rows (`AllocPrimitives(PRIM_GT4, 2)`): the blade (page 0x1C,
palette 0x1AB) and a trail retyped as LINE_G2 (orange to black, additive). Both rows sit two
above the player's priority. The entity has `FLAG_POS_CAMERA_LOCKED` and both rows have
`DRAW_UNK02`, so they keep their place in the room.

- **Birth.**
  - It takes the player's facing and rolls `func_8011A290` (attack and least-used group 7..10).
  - It takes a 4x2 hitbox four pixels ahead.
  - It rises nine pixels unless `g_Player.status` has `PLAYER_STATUS_CROUCH` (0x20).
  - It flies at eight pixels an update (`SetSpeedX`), with SWISH_C and timer 10.
- **Flight.** Each update counts up and moves one pixel at a time, eight times, probing
  `CheckCollision` eight pixels ahead.
  - The blade spans 24 pixels.
  - The trail runs from eight pixels behind the blade to `12 * (count / 2)` pixels behind it.
- **Enemy hit.** On hitFlags 1 it clears its hitbox and holds for four updates without moving or
  redrawing, then retires.
- **Wall or guard.** A blocking tile (`EFFECT_BLOCK`, including side-only walls) or hitFlags 2
  makes it bounce:
  - It clears its hitbox and reverses at an eighth of its speed, lifting at 2.5 pixels an update.
  - It requests the shared three-spark `FACTORY(10)` at the probe and plays TINK. The rows are
    not redrawn on that update.
- **Bounce.** For 64 updates it falls by 1/8 pixel per update and spins 0x80 per update with
  real `rcos`/`rsin` corners (radius 0xCA0 >> 20). The trail hides. From update 0x20 it fades
  (`DRAW_COLORS`, grey 0x60 minus two each update), then retires.

Without `KEEP_ALIVE_OFFCAMERA`, the runtime retires a dagger that leaves the screen window, as
the original does. Room entry retires it and its pending factory.

The old service's dagger code is no longer reached from gameplay. It stays until the axe, holy
water and stopwatch have moved too, and is then removed with that service.

## Validation

- **Public tests.**
  - Birth (lift, crouch, facing), the exact flight and its rows, and the left-facing mirror.
  - The wall bounce at the probe, including a side-only wall; the hitFlags 2 bounce and the
    hitFlags 1 hold (hitFlags 3 is neither).
  - The exact spin, the fade, retirement, and the runtime's rows, group, tink factory and prune.
  - A mutation sweep leaves one survivor, the corner radius 0xCA1 for 0xCA0, which changes no
    pixel even on the original tables.
- **Private emulator comparison.** The original `EntitySubwpnKnife` runs on the actual DRA/SLUS,
  with the real `func_8011A290`, `SetSpeedX` and `rcos`/`rsin`. `CheckCollision` answers come
  from the native map, including sloped and side-only tiles.
  - It covers 500 contexts and 33,726 callbacks: 387 wall bounces, 43 guard bounces, 43 enemy
    holds and 13,760 fade updates.
  - It catches a deliberate corner-angle mutation.
- **Private LLDB probe.** The probe uses the dagger in the Laboratory by pad input, facing both
  ways.
  - It hits a held zombie once and holds for four updates.
  - A fourth live dagger is refused, and every row is released.

These use explicit subweapon/hearts/protection/position/enemy-HP fixtures, not acquisition,
route or console evidence.

Adapted source: `src/dra/84B88.c` at the pinned reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
