# Selected Agunea

Agunea (subweapon 9) now runs through the shared player runtime, after
[Vibhuti](player-vibhuti.md) and [Rebound Stone](player-rebound-stone.md). Hold Up and tap
either attack button; keep Up and Square (the left-hand attack button) held to keep striking.
Defaults, grants and saves are unchanged.

## Use

The shared `func_8010EB5C` path ([Vibhuti](player-vibhuti.md#use)) with `g_SubwpnDefs[9]`:
5 hearts, chain 1, blueprint 22 (`E_SUBWPN_AGUNEA`, entity 17, slots 32..47, origin 2) and
standing animation 0x5D.

## Bolt

`EntitySubwpnAgunea`:

- **Any update.** A transformation (`PLAYER_STATUS_TRANSFORM`) or status 0x10000 destroys it,
  before allocation on the birth update.
- **Birth.** `AllocPrimitives(PRIM_GT4, 1)` retyped `LINE_G2` (a failure destroys it before any
  RNG). `FLAG_POS_CAMERA_LOCKED | KEEP_ALIVE_OFFCAMERA | HAS_PRIMS`, the player's facing, then
  `func_8011A290` (attack roll, least-used group 7..10). A 4x4 hitbox 4 pixels ahead; the bolt
  and the line's start (`unk80`/`unk82`) take the player's position plus `hitboxOffY - 8`. The
  line (priority player + 2, drawMode 0x331, additive) runs from black at the start to
  `(0x60, 0, 0x80)` at the bolt. `SetSpeedX(FIX(6))`, SWISH_C,
  `CreateEntFactoryFromEntity(self, FACTORY(BP_BLINK_WHITE, 0x52), 0)` and timer 10 = 4.
- **Flight.** Six pixels an update. Leaving the screen window (x -0x40..0x140, y -0x20..0x120)
  starts the fade. A hit binds it: the entity collision wrote to `unkB8` becomes its target.
- **Strikes.** While bound, the bolt sits on the target (its hitbox keeps damaging it under
  the ordinary per-group cooldown). Every twelfth update it jitters by two rolls and, for the
  first strike free and later ones for five hearts (Heart broach: /2, /3, at least one),
  requests the lightning (blueprint 23), THUNDER_B and `FACTORY(BP_61, 2)`: three light-blue
  player outlines five updates apart. Too few hearts, or Up+Square released, moves it to the
  follow step. A target slot that empties, or (after the first strike) has hit points 0 or
  above 0x7000 or `FLAG_DEAD`, starts the fade before the line update.
- **Follow and fade.** The follow step tracks the raw target slot (an emptied slot reads as the
  screen origin) until its count reaches sixteen; the fade retires the bolt once the line's red
  drops below five. The line's channels fade by four every update and the line hides once blue
  drops below five.

The line has no `DRAW_UNK02`, so both ends keep their screen position as the camera moves; it
does have `DRAW_UNK_200`, which in the original also moves it with a post-stage player shove
for that frame (natively omitted: lifts only).

## Lightning

`EntityAguneaHitEnemy` (entity 18, slots 32..63 from the top, factory parent = the bolt's slot,
subweapon id zero so `CheckSubwpnChainLimit` never counts it):

- It sits on the player (`hitboxOffY - 8`) every update. If its raw parent slot no longer holds
  an Agunea it is destroyed at birth and otherwise jumps to the fade.
- **Birth.** `AllocPrimitives(PRIM_GT4, 40)`: page 0x1A, palette 0x194, columns 0x90.. cycling
  every six rows, grey 0x80, priority = the bolt's. One roll sets the start angle (the original's
  facing adjustment is discarded). Row 0 stays hidden and anchors the chain at the player.
- **Growth.** Two segments an update: steer toward the parent through `ratan2` (a quarter of the
  turn each four segments; half each two within 64 pixels), wobble with one roll per segment
  when far, then extend a 12-pixel segment 16 pixels wide (`DRAW_COLORS | DRAW_UNK02`). It stops
  within eight pixels of the parent or at row 39.
- **Flash and fade.** One update with `PAL_FILL_WHITE`, then grey 0x60 fading by four
  (`PrimDecreaseBrightness`, clamped at 16) until every row is spent. It has no hitbox.

`BlinkWhite` parameter 0x52 (profile 0x12, hold 0x7003) stays lit while any entity 17 lives in
slots 32..47. Room entry retires the bolt, its lightning and their pending factories.

## Validation

Public tests cover the bolt's transformation exit, birth, flight, window exit, binding, free and
paid strikes, refusal, release, emptied/dead/high-HP targets and line fade; the lightning's birth
roll, growth, row limit, reach, flash, fade and lost parent; the runtime's hit target, raw parent
slot, heart payment, screen-anchored rows, blink hold and room cleanup; and the 0x52 blink hold.
A private emulator comparison runs the original `EntitySubwpnAgunea` and `EntityAguneaHitEnemy`
(with the real `func_8011A290`, `ratan2`, `rcos` and `rsin`) on the actual DRA/SLUS against the
native callbacks over 400 contexts with nonzero scroll; deliberate mutations are caught. A
private LLDB probe throws Agunea in the Laboratory by pad input both ways, binds a positioned
zombie and strikes it. These use explicit subweapon/hearts/protection/position/enemy-HP
fixtures, not acquisition, route or console evidence.

Adapted source: `src/dra/84B88.c`, `7879C.c`, `6E42C.c` and `4DA70.c` at the pinned reconstructed
revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
