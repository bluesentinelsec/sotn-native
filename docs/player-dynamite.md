# Selected Dynamite

Dynamite (item138) is a kind130 consumable running the weapon47 program (`w_047`) in either
hand. Each press throws one stick: it lands, fizzes, and either explodes or (one throw in eight)
fizzles out. Defaults, grants and saves are unchanged; the stock is ordinary inventory.

## Use

Kind130 takes the existing throw path:

- CheckChainLimit (chain one; the live root carries item138);
- the blueprint50 factory (E0/F0 in slots17..31, with the hand bit);
- the casting grunt and animation 0x60;
- one consumed stick after the player's successful update (Duplicator keeps it, as for the other
  thrown items).

## The stick

`EntityWeaponAttack` (role0) at birth:

1. `SetSpriteBank1`, then the hand's animSet (0x8010/0x8012), unk5A (0x65/0x67) and palette
   (0x110/0x128). For the right hand, unk88 is 0x80.
2. `LoadImage` of the program's 4x16 words `D_14D000_8017A4A8` into the hand's weapon texture, at
   x0x260, y0x100 (left) or 0x180 (right). The session commits the patch after a successful pass.
3. The player's facing, zPriority-6 and CAMERA_LOCKED.
4. `rand()&7`. Only when it is non-zero does the stick call `AllocPrimitives(TILE, 32)`. These
   fuse sparks are optional: failure leaves a dud. Each spark takes colour (0x80,0x40,0), a 1x1
   size, two rands (`x1` offset index and `x3` start delay), priority-4 and a hidden drawMode0x3B.
   The runtime reserves the rows for the birth callback and releases them at once when the roll
   declines them.
5. velocityY -2.5 and `SetSpeedX(rand()+1.5)`. Its height is the player's plus hitboxOffY -8, and
   a further -6 unless crouched.
6. `DestroyEntityWeapon(true)` (a raised shield retires), SWISH_C, fuse 0x80, the program's stomp
   flag (`D_14D000_8017B6A4`) cleared, and `g_Player.timers[10]` = 4.

Each update it shows frame1, moves, and gains 1/8 gravity up to 7.0. A contact (hitFlags) slows
it to two thirds and skips the rest of that update. Three CheckCollision probes follow:

| Probe | Point | On hit |
| --- | --- | --- |
| Side | x±4 (direction of travel) | Corrects by the tile edge; reverses at half speed |
| Ceiling | y-4 | Bottom+1 correction; velocityY 1.0, half X speed |
| Floor | y+4 | Top correction; stops. The program's first landing plays STOMP_SOFT_A |

Sparks play RAPID_SCRAPE_3X on every eighth game update. A spark whose delay has elapsed is placed
from `D_14D000_8017A468/A488[x1] * x2 / 15` beside the stick (±8, mirrored when facing left). It
advances `x2`, rerolls `x1` after fifteen, and shows. The TILE's x1..x3 words hold this scratch,
exactly as in the original.

## Explosion

On the fuse's 0x80th counted update, a stick with sparks:

1. `ShakeCamera(SHAKE_Y_HEAVY)`.
2. Hides its rows and converts them to GT4 rays: UVs from `rcos/rsin(i<<7)` around (0xC0,
   0x40+unk88), PAL_FILL_ORANGE, tpage0x19, priority player+8, black centres and white rims.
3. EXPLODE_A, a 4x4 hitbox, `SetWeaponProperties` and then `func_80118894` (the least-used group
   twice).

Each later update:

- The hitbox grows by six up to 0x60.
- The radius grows by 0x480/256 to 128.
- While the first ray's brightness exceeds 0x58, it requests a 0x3A flame from its own position.
- Each ray fades by four. Below five, the explosion retires: 63 drawn updates, 42 flames.

A dud instead blinks (ENTITY_BLINK) for 0x18 updates and retires.

Rows are world pixels: the original `DRAW_UNK02` primitives follow camera scroll. Room entry
translates the TILE x0/y0 (never the scratch) or every ray vertex.

## Flames

`func_ptr_80170004` (role1, blueprint0x3A, slots32..63) checks its raw parent slot's entityId at
birth: an empty slot destroys it. It then:

- calls `SetSpriteBank2` (0x8017A0D4) with the hand's animSet (0x8011/0x8013), unk5A
  (0x64/0x66) and palette (0x111/0x129);
- plays `D_14D000_8017A528` with CAMERA_LOCKED|0x100000 at priority player+2;
- takes heading `rand()%1535+0x100` (velocity `rcos/rsin<<7`) and moves two steps;
- picks BLEND_TRANSP|ADD, or QUARTER when `rand()&1`.

It moves every update and retires on the callback after its animation's END.

## Evidence

Public tests cover:

- Spark/dud rolls, exhaustion, both hands, facing and crouch, with the birth state.
- Flight with gravity, contact slow-down, wall bounce, one stomp, the scrape cadence, and spark
  placement and re-rolls.
- The 0x80-update fuse, the explosion's UVs and events, ray growth, flames, fade and retirement,
  and the dud blink.
- The flames' parent check, heading, blend, motion and END retirement.
- A shared-runtime throw in both hands (real rows, released rows for duds, group, flames' raw
  parent and slots, texture request) and the player's consumption.

A private MIPS oracle executes both original callbacks over 7,210 root and 435 flame traced
callbacks (explosions, duds, failed allocations, bounces, contacts, shield retirement, the
texture patch, missing flame parents and END retirement). Debug and ASan traces are
byte-identical.

A private native LLDB probe in Laboratory room 3 throws real Dynamite from the equipment menu in
both hands, by actual pad input. It saw nine explosions (32 drawn rays, 42 drawn flames, camera
shake, enemy kills) and a dud. Every throw consumed one stick and patched the texture. These
fixtures supply items, stock, protection and positions; they are not acquisition, route or
console evidence.

Adapted from `src/weapon/w_047.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under
AGPL-3.0-or-later. All original definitions, tables, palettes, texture words and sounds remain
private.
