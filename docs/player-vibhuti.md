# Selected Vibhuti

Vibhuti (subweapon 8) now runs through the shared player runtime, the same owner as Alucard's
weapons, instead of the older bounded subweapon service. Hold Up and tap either attack button.
It is the first subweapon moved onto that runtime; dagger, axe, holy water and the stopwatch
still use the older service until they move too. Defaults, grants and saves are unchanged.

## Use

`func_8010EB5C` with the original definition (`g_SubwpnDefs[8]`: 3 hearts, chain 2, blueprint 26,
standing animation 0xBC):

- `func_800FE3C4`'s heart check (Heart broach discounts, minimum one); too few hearts falls
  through to the ordinary hand attack.
- `CheckSubwpnChainLimit`: two live Vibhuti entities in slots 32..47, or no empty slot there,
  refuses the use (also falling through to the hand attack).
- The hearts are spent, then `CreateEntFactoryFromEntity(player, 26, 8 << 9)` reserves a
  factory. The original ignores a missing factory: the hearts stay spent.
- `timers[ALU_T_USE_SUBWPN] = 4`, the standing animation, then `func_8010EA54(8)`: one libc roll
  and a grunt below four (skipped when `step_s >= 0x40`). Natively a failed player update
  restores the spent hearts, the roll and the factory.

## Entity

Blueprint 26 creates `E_SUBWPN_VIBHUTI` (entity 12) at the player's use position in slots
32..47; the factory child's ext `subweaponId` (0xB0) is 8. `EntitySubwpnThrownVibhuti`:

- **Birth.** `func_800EDB58(PRIM_TILE_ALT, 13)` takes thirteen *contiguous* rows (twelve ash
  particles and a zero-width terminator); without them the entity is destroyed before any RNG.
  `func_8011A290` then rolls `func_800FE3C4`'s attack (Brilliant mail, `(INT*2 + rand%12)/10`)
  and `func_80118894` picks the least-used group 7..10. Each particle takes two rolls (angle
  0x100..0x1FF, speed 0x80..0x17F) through `rcos`/`rsin`, starts 12 pixels above the entity,
  and is a 2x2 opaque white TILE at the player's priority + 2. SWISH_C and timer 10 = 4.
- **Updates.** For 128 updates each falling particle moves, stops horizontally on a side
  wall (probe 2 pixels ahead), falls with gravity 12/128 up to 4 px/update and lands on a solid
  floor as a 3x3 TILE 1..3 pixels above its surface. The 4x4 hitbox follows particle
  `timer & 7` (only particles 0..7 ever carry it), lifted 6 pixels once that particle has
  landed, and is zero-sized while it is hidden. A hit hides the previous host particle; the
  last twelve updates hide one particle each.
- Hidden rows lose `DRAW_UNK02`, so they keep their screen position while the camera moves.
  The entity lacks `KEEP_ALIVE_OFFCAMERA`: it is destroyed when its hitbox particle leaves the
  screen (for example ash thrown off a room edge). Room entry retires it (no `FLAG_UNK_20000`).
- `UpdatePlayerEntities` clears the hitbox when the player dies or teleports; Vibhuti never sets
  it again.

## Validation

Public tests cover the birth RNG/velocities, a failed reservation, landing, the wall stop, the
hitbox cycle, hit/expiry hiding, contiguous rows, the runtime factory, group, power, combat mask,
freeze, pruning and the player's use/grunt/fallthrough. A private emulator comparison runs the
original `EntitySubwpnThrownVibhuti` and the real `func_8011A290` on the actual DRA/SLUS against
the native callback over 400 contexts (allocation, collision, sound and destruction adapters).
A private LLDB probe throws Vibhuti in the Laboratory by pad input in both directions. These use
explicit subweapon/hearts/protection/position fixtures, not acquisition, route or console
evidence.

Adapted source: `src/dra/6E42C.c`, `5D5BC.c`, `7879C.c`, `84B88.c` and `4DA70.c` at the pinned
reconstructed revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
