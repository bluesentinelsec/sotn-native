# Sword familiar component

The Sword familiar (original `servant/tt_004`, card relic 22, active familiar 5, stats record 4)
now has a native component (`modules/core/sword_familiar.c`) for its idle side. It reconstructs
`ServantInit`, the default follow, the menu-switch entrance, the GTE-projected 3D model, the blade
trail, the level-90 halo, the room-event entity and the narrative sound cursor. Its private model,
UV, CLUT and event tables are read from the user's `TT_004.BIN`.

It is a component, like the earlier Fairy components: no native card selection, resource loading,
primitive leases, drawing or room ownership uses it yet. Familiar levels already persist in v11
saves ([native saves](native-saves.md)). Attacks, Sword Brothers, the level-50 gift and the
Sword Familiar item follow in later batches; until then those states fail explicitly instead of
degrading.

## Behaviour

- **ServantInit** (modes 0 engine, 1 menu switch, 3 same servant): mode 3 only resets the room-event
  cache. Otherwise it requests the 48-word CLUT upload to row 0xF4 (`g_ServantClut` then
  `g_SwordClut`, palettes 0x140–0x142) and replaces slot 4. A menu switch outside flag-20/40 rooms
  starts the entrance at screen (128,-32). Elsewhere it creates the default sword: in flag-20 rooms
  at the side `ServantUnk0` picks (reversed in the inverted castle), otherwise 28 pixels behind the
  player. The sword faces away from the player. A menu switch in a flag-20/40 room turns castle
  flag 1 or 2 into 4. The level row (`D_us_801700A0[level/10]`) spawns the blade trail (bit 0) and,
  from level 90, the halo (bit 1) into the first empty slot 5..7.
- **Default sword**: first update reserves 96 GT4 rows (failure retires it), then sets up the small
  (72 faces, below level 50) or large (96 faces) model. It then follows a target 28 pixels behind
  the player, bobbing with `rcos`. In flag-20 rooms it follows a fixed screen side instead. Speed
  depends on distance: a quarter of the gap above 256 px, then 1/16, 1/32 and 1/64 below 16 px. It
  yaws toward its heading (step 8) and pitches with its model depth (step 0x20), and the depth drifts
  by `rsin(pitch)*0x60`. A live target, Sword Brothers or the level-50 gift currently fail
  explicitly.
- **Entrance**: approaches the player more slowly and plays its greeting once within 32 px. The
  greeting is XA 0x4FB on the first summon, 0x4FE after the level-50 gift, 0x4FC below level 50,
  otherwise 0x4FF. Pause is disabled until the speech ends. Player movement, enemies, cutscenes,
  global flashes or the room event 0 abort to the default sword.
- **Model**: the overlay's own rotation matrix, `TransMatrix(position/5)`, `CompMatrix` with
  (0,0,6500), RTPS at offset (128,128) and screen 1000, and NCLIP back-face culling (zero area
  stays visible). Each face's priority comes from the farthest vertex's OTZ (SZ3/4) relative to the
  nearest vertex. The layer is 0x9F/0x93/0x8B by depth sign, or near the player after room event 15.
  The blade edge, halo quads and tips are projected for the trail and halo.
- **Trail**: sixteen dithered additive bands shift along the projected blade edge; a stale edge hides
  them.
- **Halo**: four additive quads at level 90+, coloured from a phase that advances by 0x60 per update.
  The second phase never advances, as in the original.
- **Room events** use the shared owner with the Sword's 49-record table at 0x801719E4. Event 0
  stops a live speech; event 15 raises the layer. The sound cursor is the shared `ProcessSfxState`.

`DestroyEntity` after a failed model allocation only returns from the setup, so the original keeps
projecting from the zeroed entity and updates the globals. Native does the same. It omits the
original's writes through the dead entity's zero `primIndex` into another owner's packets. A
distance beyond about 724 px (`SquareRoot12` input above `INT32_MAX`) fails explicitly. Neither
case arises in the traced play.

## Evidence

- Public tests (synthetic tables) cover:
  - init placement for every mode, room kind and castle;
  - the entrance speech choices, pause and aborts, its speed bands and the 32 px settle;
  - follow speed bands and an exact follow-step model (velocity, heading/yaw, pitch, depth);
  - an independent model of the matrix, RTPS, UNR divide, NCLIP and priorities, compared against
    every projected word and row of both models;
  - degenerate faces, the trail shift and colours, halo colours and phase;
  - room events, aura respawn on a level change, explicit failures, allocation failures,
    camera/room translation and clear.
  58 of 60 mutations are caught; one does not compile and one changes the unused `rotation[1]` term.
- The private comparison runs the original `ServantInit`, `UpdateServantDefault`, the entrance,
  trail, halo, event entity and `ProcessSfxState` in lockstep with the native component. It uses
  SLUS `rsin`/`rcos`/`ratan2`/`SquareRoot12`/`CompMatrix` (SquareRoot12's leading-zero count
  adapted) and the private PSY-Z COP2 registers. It covers 8 contexts and 5,135 callbacks with
  263,040 row comparisons and 353,538 COP2 commands. Every slot, global and row matches, including
  the CLUT upload, speech and castle-flag writes. Debug and ASan traces are byte-identical.
