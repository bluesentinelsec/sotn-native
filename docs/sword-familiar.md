# Sword familiar component

The Sword familiar (original `servant/tt_004`, card relic 22, active familiar 5, stats record 4)
now has a native component (`modules/core/sword_familiar.c`) for its idle side. It reconstructs
`ServantInit`, the default follow, the menu-switch entrance, the GTE-projected 3D model, the blade
trail, the level-90 halo, the room-event entity and the narrative sound cursor. Its private model,
UV, CLUT and event tables are read from the user's `TT_004.BIN`.

The native session now owns it through the familiar load boundary (see [Session
integration](#session-integration)). The relic menu does not offer the Sword Card yet. Familiar
levels persist in v11 saves ([native saves](native-saves.md)). Attacks, Sword Brothers, the
level-50 gift and the Sword Familiar item follow in later batches. Until then those states fail
explicitly instead of degrading, so a selected Sword stops the game once an enemy is on screen.

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
- **Targets**: `func_us_80173AA0` finds a target exactly when `CheckAllEntitiesValid` finds an
  enemy, so the session passes that predicate. The component then stops explicitly until attacks
  exist. Sword Brothers stays false because no native spell sets `g_Player.unk70`.
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
- The private comparison runs the original `ServantInit`, `UpdateServantDefault`, the entrance,
  trail, halo, event entity and `ProcessSfxState` in lockstep with the native component. It uses
  SLUS `rsin`/`rcos`/`ratan2`/`SquareRoot12`/`CompMatrix` (SquareRoot12's leading-zero count
  adapted) and the private PSY-Z COP2 registers. It covers 8 contexts and 5,135 callbacks with
  263,040 row comparisons and 353,538 COP2 commands. Every slot, global and row matches, including
  the CLUT upload, speech and castle-flag writes. Debug and ASan traces are byte-identical.
