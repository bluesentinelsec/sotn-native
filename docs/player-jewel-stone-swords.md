# Selected Jewel sword and Stone sword

The Jewel sword (item109) and Stone sword (item114) share the Short sword's weapon37 program
(`w_037`). They run in slot16 through the shared ordinary hand owner, in either hand. The Jewel
sword's QCF special198 adds gold coins to its sparkles. Defaults, grants and saves are unchanged:
gold already belongs to ordinary character progress.

## Swings

Each item's definition must match its supported US path: family37, kind15, animation0xA7.
Descriptors at 0x6A4 select the variant:

| Item | Variant | Notes |
| --- | --- | --- |
| Short sword19 | 0 | [Unchanged](short-sword.md) |
| Jewel sword109 | 1 | Sparkles |
| Special198 | 2 | Sparkles with coins; five MP |
| Stone sword114 | 3 | Its stone element (0x200) comes from its definition |

Variants1 and2 request `WFACTORY(0x3A, variant-1)` once, on the first pose1 of a swing; the
per-hand previous-pose word stays in program BSS. Special198 is the Jewel sword's equipped
specialMove. A completed grounded QCF selects it when its chain limit and MP allow; it costs five
MP after the factory, as the existing [special input owner](player-weapon-specials.md) does.

Native generic combat has no element-specific enemy flash for any weapon. The Stone sword therefore
deals its stone-element damage like the other elemental weapons; no petrification effect is added.

## Sparkles

Blueprint0x3A creates the E1/F1 sparkles (`func_ptr_80170004`) in slots32..63. The special
(params1) first rolls `rand()&31` into the program's writable coin pattern (`D_107000_8017A758`):

| Roll | Marked sparkles |
| --- | --- |
| zero | all |
| `&3==0` | every third, plus every fifth |
| `&3==3` | every ninth |
| otherwise | every fifth |

The sparkles then try to allocate sixteen GT4s. Failure destroys them after the roll. The entity
takes FLAG_HAS_PRIMS|POS_PLAYER_LOCKED|0x20000 and its origin from `func_107000_8017B0AC`. That
origin is the player plus the reach row `D_107000_8017A714[anim-0xA7]`, mirrored when facing left.
A DRAW_UNK02 (rotated) pose lifts it three pixels.

Eight sparkles spread along half the row's line. Each scatters by `rand()%10` down and then
`rand()%4` right. Each takes `D_107000_8017A74C[counter%6]` from the program's running counter,
tpage0x1A, priority+4 and drawMode0x339; `rand()&1` picks 0x379 instead. Sparkle *i* waits *i+1*
callbacks. It then draws six `func_107000_8017ADF8` frames (6x6, then 4x4 from frame3), advancing
every second update and rising one pixel per update. Frame6 hides it. A marked sparkle then requests
`WFACTORY(64,0)` from its last point. The entity retires after thirty-two callbacks.

The points, pattern and counter live in the program, so overlapping sparkle entities share them as
the original does. Rows are screen pixels; room entry translates them once.

## Coins

Blueprint0x40 creates the E2/F2 coins (`func_ptr_80170008`) in slots32..63. Their raw parent is a
sparkle, so weapon37 secondaries accept parent slots up to 63 (every other family keeps 16..47).
A full factory table drops a coin request, as `CreateEntFactoryFromEntity` returning NULL does.

A coin born inside a solid tile is destroyed before its rand calls. Otherwise it plays
`D_107000_8017A798` from DRA bank3, sits two priorities in front of the player with
CAMERA_LOCKED|0x100000, and launches at `SetSpeedX(rand()+0.25)` and `-(rand()+1)` with 9/64
gravity. Each update it probes, in order:

1. The ceiling (y-1): bottom correction, velocityY 1.0.
2. The right side (x+4): left correction, reversed velocityX.
3. The left side (x-4): right correction, reversed velocityX.
4. The floor (y+7): top correction, then a 0x50-update rest with GOLD_PICKUP.

A resting coin blinks from 0x10 and retires at zero. In every step, touching the player
(|dx|<0xB, |dy|<0x1B) pays one gold up to 999999, plays GOLD_PICKUP and retires the coin. A
successful shared-runtime tick commits the gold; a failed one keeps the old total.

## Evidence

Public tests cover:

- All four roll classes, exhaustion, placement, palettes, the shared counter and waits.
- Frames, rising, hiding, coin requests and retirement.
- The coins' birth-in-solid destruction, launch, ceiling and wall bounces, landing, blink, expiry
  and pickups.
- A shared-runtime special198 swing leasing sixteen real rows, coins in slots above47 and gold.

A private MIPS oracle executes both original callbacks over 3,241 sparkle and 3,851 coin traced
callbacks. The traces cover all roll classes, rotated poses, exhaustion, overlapping pairs,
ceilings, walls, landings, blinks, pickups, the gold cap and births inside walls. Debug and ASan
traces are byte-identical.

A private native LLDB probe in Laboratory room 3 uses actual pad input and the equipment menu, in
both hands:

- Jewel swings draw their sparkles without coins or MP.
- A real QCF special198 costs five MP; its sparkles drop drawn coins, and walking onto them pays
  gold.
- The Stone sword hits a positioned enemy.

Those fixtures supply items, protection and positions; they are not acquisition, route or console
evidence.

Adapted from `src/weapon/w_037.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under
AGPL-3.0-or-later. All original definitions, tables, palettes and sounds remain private.
