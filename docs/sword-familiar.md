# Sword familiar component

The Sword familiar (original `servant/tt_004`, card relic 22, active familiar 5, stats record 4)
now has a native component (`modules/core/sword_familiar.c`) for its idle side. It reconstructs
`ServantInit`, the default follow, the menu-switch entrance, the GTE-projected 3D model, the blade
trail, the level-90 halo, the room-event entity and the narrative sound cursor. Its private model,
UV, CLUT and event tables are read from the user's `TT_004.BIN`.

The native session owns it through the familiar load boundary (see [Session
integration](#session-integration)). The component also reconstructs the target search and both
attacks; the session does not connect them yet. The relic menu does not offer the Sword Card yet.
Familiar levels persist in v11 saves ([native saves](native-saves.md)). Sword Brothers, the
level-50 gift and the Sword Familiar item follow in later batches. Until then those states fail
explicitly instead of degrading. A selected Sword therefore still stops the game when it starts an
attack.

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
  by `rsin(pitch)*0x60`. Outside cutscenes it searches for a target every update and attacks one
  if found. Sword Brothers and the level-50 gift currently fail explicitly.
- **Target search** (`func_us_80173AA0`) scans stage slots 64..191. It picks the first entity that
  has a hitbox, lacks flag 0x200000, sits on screen (x −16..272, y 0..240) and has HP below
  0x7000, provided it is not yet marked; it marks that entity with flag 0x80000. If every
  candidate is marked, it takes the next one after a round-robin cursor. The search does not check
  HP above zero; the attacks' later `CheckEntityValid` does. The target is a raw slot: a new
  entity in that slot inherits the attack.
- **Circle attack (D2, below level 70)**: two `rand()` calls pick a point 64..191 pixels into the
  screen. The sword turns toward it (step 32), flies there at 1/16 of the gap until within 2
  pixels, then holds for 30 updates. It plays `0x672` and spins twice (128 per update), slashing
  every update, then holds 30 more. Finally it keeps a valid target (or finds another) and starts
  over, or returns to the default sword on a cutscene or with no target.
- **Dive (D3, level 70 and above)**: two birth `rand()` calls. It rises half a screen-unit behind
  the player's facing and settles (60 updates). Then, with `0x6C6`, it flies in 3D past a
  corner, glides 15 updates, homes on the target's screen position, glides again and repeats,
  re-checking the target before each pass. It slashes only while its depth is within ±0x200.
  Steering uses `ratan2`, `rcos`/`rsin` speeds of 5/128 or 3/64, pitch steps, and `SquareRoot0`
  of the 3D distance (below 0x200 to arrive).
- **Slashes**: the first slash of each swing rolls `rand()%16`; 0 or 1 plays a voice from the
  overlay's table. Every slash then requests two DE hitbox entities from slots 17..46
  (`GetFreeEntity(0x11, 0x2F)`). Each takes the projected blade tip, a 4×4 hitbox and spell 19
  (circle) or 20 (dive) through `GetServantStats`, hits for one hit detection and is destroyed on
  its next update.
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

## Session integration

`SotnLaboratory_SelectFamiliar(session, 5)` and engine initialization now load the Sword like
the Fairy, through one per-familiar profile: card relic 22, stats record 4, the
`TT_004`/`FT_004`/`SD_SWORD` resources and the event table at 0x801719E4.

- **Loading**: the FT pages go to 0x2C0,0x100 and 0x2C0,0x180, like every familiar. The CLUT
  upload writes only `ServantInit`'s 48 words to row 0xF4; the rest of the row keeps its colours.
  The model draws from GT4 rows, so the Sword has no bank-20 sprite binding. The SPU bank and CD
  owner follow the Fairy's detached-candidate path, and every failure preserves the live owners.
- **Overlay residency**: one servant overlay is resident at a time. Selecting a different familiar
  replaces the cached resources, and the newly loaded overlay's data starts fresh: the sound
  cursor returns to 99 and the event history resets. Reselecting the cached familiar keeps its
  resources and history. Deselecting retires slots 4..7 and their rows, but keeps the overlay.
  Connected-stage handoffs carry the resident overlay's runtime, with its slots and rows retired.
- **Row leases**: each callback that calls `AllocPrimitives` first checks the real stage pool:
  96 rows for the model, 16 for the trail, 4 for the halo. A failed check retires the owner, as in
  the original. After every callback the lease each part holds is committed atomically, tagged
  by part and slot. Kept owners keep their rows, retired owners release theirs, and stale or
  unclaimed rows fail before anything changes.
- **Callbacks**: the ordinary pass runs slots 4..7 after the Fairy's. In the spell-freeze pass only
  flag-0x10000 entities update, and the Sword has none. The session supplies the following, and
  the callback's speech, pause and castle-flag 464 requests are committed after it:
  - the screen-space player and the record-4 stats;
  - the room's 0x20/0x40 flags, the cutscene and global-flash flags, and enemy presence;
  - real CD acknowledgements.
- **Targets**: the session snapshots stage slots 64..191 in screen space; empty slots read zero,
  as `DestroyEntity` leaves them. It passes the character's libc `rand()` state and the tilemap
  origin. The search's mark, the DE hitboxes and the attack voices need stage-flag and
  shared-slot owners that are not connected yet, so the session stops explicitly when an attack
  starts instead of losing them. Sword Brothers stays false because no native spell sets
  `g_Player.unk70`.
- **Translation**: camera scrolls move camera-locked entities and their DRAW_UNK02 rows. Room entry
  retires non-0x20000 entities and moves the rest, with DRAW_UNK_100 rows, by the original player
  displacement. It then releases the rows of retired owners.
- **Drawing**: the session adds visible model, trail and halo rows to the ordered draw list at their
  own priorities. It draws them as textured GT4 packets (page 0x1B, palettes 0x140/0x141) with the
  GPU's wrap rule for private-table packets.

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
- Public lease tests cover:
  - the row requests and held parts;
  - two-owner rejection;
  - pool commits that keep, replace and release rows;
  - stale-row and unclaimed-row holds;
  - exhaustion and malformed requests.
  17 of 18 mutations are caught. The survivor is equivalent: a model owner with kind 0 is always
  at step 0.
- A private native probe runs Laboratory room 16 through these steps:
  1. A selection without the card holds.
  2. Card 22 is granted, and the Sword is selected through the API.
  3. Over 283 frames, the session runs 644 slot-4/5 callbacks with six real row reservations.
     The visible model changes the frame, and blanking the 48 CLUT words changes it again.
  4. Walking scrolls the camera, and the probe crosses into the neighbouring room. The model
     lease stays unchanged throughout.
  5. Deselecting retires every row.
  6. Cached reselection increments the load count to 2 without new resources.
  7. Switching to the Fairy replaces the overlay and retires all Sword rows. Switching back
     reloads a fresh overlay (loads 3).
  8. A stale row stops the next load.

  These are explicit card and selection fixtures in an enemy-free room. They are not menu
  activation, attack or route evidence. The probe's save-room variant stops explicitly when the
  next room shows enemies.
- Public attack tests cover:
  - search eligibility edges, the round-robin cursor with wrap, cutscene holds and D2/D3 choice;
  - the circle's rand point, turn step, 1/16 approach and 2-pixel arrival, hold and sweep sound;
  - its 64 slash updates with tip hitboxes, spell 19 and one voice roll per swing (table order);
  - re-targeting through HP 0, cutscene and no-target exits, and the leased model;
  - the dive's birth rands, its fall-through rise, speed scaling, 0x200 arrival, the wing sound,
    homing, depth-gated slashes with spell 20 and its position write-back.
  27 of 28 attack mutations are caught; the survivor is equivalent (the attack setup's camera
  origin is set again before every update).
- The private comparison runs the original `ServantInit`, `UpdateServantDefault`, the entrance,
  trail, halo, event entity, `ProcessSfxState`, the target search, both attacks and each DE
  hitbox's first update in lockstep with the native component. It uses SLUS
  `rsin`/`rcos`/`ratan2`/`SquareRoot12`/`SquareRoot0`/`CompMatrix` (the square roots' leading-zero
  count adapted), the libc `rand()` and the private PSY-Z COP2 registers. It covers 12 contexts and
  13,636 callbacks, including:
  - 3,372 attack updates and 1,206 hitbox entities;
  - 6 target marks, 19 voices and 35 `rand()` calls;
  - 692,240 row comparisons.
  Every slot, global, row, mark, cursor, sound, rand state and hitbox matches. The hitbox
  comparison covers position, parameter, spell, layer and facing. The attack contexts include
  targets that move, die, start marked or stay ineligible, and a cutscene interruption. Debug and
  ASan traces are byte-identical.
