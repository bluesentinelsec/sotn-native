# Ghost familiar

The Ghost familiar (`tt_001`, Ghost Card) now runs in gameplay. Switch on an owned Ghost Card
in the relic menu; closing the menu summons the ghost. It follows Alucard, flies to nearby
enemies and attacks them, steals souls from level 70 and gains EXP from kills.

Defaults, grants and saves are unchanged: nothing grants the Ghost Card, and the Ghost's level
already lives in the v11 familiar records (record 1).

## Session

The session uses the same familiar load path as the [Sword](sword-familiar.md):

- **Loading.** The Ghost profile is servant 2, card 19, record 1, `TT_001`/`FT_001`/`SD_GHOST`
  and the room-event table at 0x801705B0. One familiar overlay is resident; switching replaces
  it. The prepared graphics bind the overlay's bank-20 sprite parts (0x80170040). ServantInit's
  256-word CLUT then replaces the raw upload at row 0xF4 before the candidate commits.
- **Updates.** Slots 4..7 run in order on ordinary passes. Before each callback the session
  checks the real stage pool for the ring's eight or the question marks' three rows. After it,
  the session commits the leases (namespace `0xA000 | slot`), the target mark and the glass
  sound.
- **Translation.** Camera movement and connected-room entry translate the ghost and its rows
  the same way as the Sword's.
- **Drawing.** The ghost draws its bank-20 sprite (palette 0x140, `unk5A` 0x6C) at its pulsing
  opacity with additive blending. The ring and question marks draw as their GT4 rows.
- **Combat.** An attack calls `GetServantStats`. It rolls the spell (one character `rand`, INT,
  Mojo Mail), scales it by `level * 4 / 95 + 1` and takes `func_80118894`'s least-used group
  3..6. The group stays counted until the pass's slot 17..47 recount.
  - The ghost's own slot-4 hitbox (8x8) is then live for exactly one hit detection. It is
    checked before slots 16 and up, as `HitDetection` scans attackers in slot order.
  - Spell 18's hit effect 2 creates real soul-steal orbs. Kills give familiar EXP through the
    existing reward path.
  - Death, a room change, ServantInit and deselection retire the box.

## Behavior

`SotnGhostRuntime` holds the four servant slots (4..7), the children's rows and the overlay's
globals. The native owner supplies real row allocation outcomes and commits the requests.

- **ServantInit.**
  - Engine modes (0/2) place the ghost 18 pixels behind and 32 above the player. In a flag-20
    room it takes the side `ServantUnk0` picks, at (64 or 192, 160).
  - A menu switch (1) starts it above the screen at (128, -32).
  - Mode 3 (same servant) changes nothing.
  - **CLUT.** The upload copies 256 words from the 48-word `g_ServantClut` (setting 0x8000 except
    on every sixteenth word) and 32 words from the 16-word `g_GhostClut` at index 0x30.
    - Both copies run past their tables into the overlay data that follows: the animation and
      ability tables, the question-mark offsets and the first room-event records.
    - The event records' timer and next words are copied as they currently stand.
- **Follow (step 1).**
  - The ghost bobs (`rsin >> 10`) toward the follow point and turns at most 512 units an update.
  - Its speed scales with the `SquareRoot12` distance, with a quarter-pixel floor; beyond 256
    pixels it snaps.
  - It faces the player's original rules and pulses its opacity between 0x20 and 0x80.
- **Target search.** `FindValidTarget` scans stage slots 64..191 for live, on-screen targets
  within 48 pixels in front of the ghost.
  - Shielded targets (hitbox state 8) qualify only at levels 1..39.
  - The first unmarked target receives `FLAG_UNK_80000`. Otherwise the search continues
    round-robin from the cursor over already-marked ones.
  - A cutscene suspends the search.
- **Attack (steps 2/3).**
  - The ghost closes in until it is within eight pixels.
  - At the level's `delayFrames - 30` (120 at levels 1..19 down to 60 from level 80) it requests the
    ring child. When the delay ends, `GetServantStats` applies spell 17, or spell 18 (soul steal)
    from level 70, and gives the ghost an 8x8 hitbox for that one update.
  - Leaving reach releases the ring at once. A dead or off-screen target returns the ghost to
    following.
- **Ring (0xDA).**
  - Eight GT4 sparkles (page 0x1A, palette 0x143, additive) spiral into the ghost over thirty
    updates. They play the glass-shards sound (0x6B0).
  - Its timer and angle are overlay globals, as in the original.
- **Confusion (steps 4..6).** While the player is a bat or in Axe Armor:
  - the ghost hovers and turns four times;
  - it shows three question marks (0xDB, unhidden ten updates apart);
  - it then rises off the top of the screen and parks at (128, -16).
  - Leaving that form releases the question marks and resumes following.
- **Room event.** The shared event table's only Ghost record is the unlit spike room
  (41, 50). It requests `DestroyServantEntity` (0xDF, parameter 15), which keeps the ghost's
  priority two below the player's.
- **Animation.** The eight-frame loop runs through DRA's `UpdateAnim` rules.

### Original quirks kept

- `CreateChildEntity` returns zero (the original NULL) when slots 5..7 are full. The later release
  check then reads PS1 low memory, which never holds the ring's identity; natively the zero link
  never matches.
- Child links are raw slots without generations, as the original pointers are.
- A room entry before the ghost's first update retires it: it has no `FLAG_UNK_20000` yet.

## Validation

- **Public test** (synthetic overlay and SDK tables). It covers:
  - every init mode and the CLUT copy past both tables;
  - the follow pulse and frame loop;
  - marking, facing and shield filters, the ring at frame 30, the spell-17 attack and its
    one-update hitbox;
  - the ring's release and allocation failure, soul steal, the cutscene;
  - confusion, the question marks, the rise and park, Axe Armor;
  - the room event, translation, retirement and failure paths.
- **Private emulator comparison.** The original `ServantInit`, `UpdateServantDefault`,
  `UpdateAttackEntities`, `UpdateConfusedEntities` and `DestroyServantEntity` run on the actual
  TT_001/DRA/SLUS with the real `UpdateAnim`.
  - 9 contexts, 7,489 callbacks, 8,801 rows, 14 marks, 35 attacks (24 spell 17, 11 spell 18),
    37 rings, 5 failed allocations and the room event all match. This covers all seven steps,
    the CLUT and the overlay globals.
  - Seven mutants fail it.

- **Public tests.** The Ghost's row leases (first free rows, kept owners, stale/unclaimed rows,
  malformed requests and exhaustion fail without change) and `GetServantStats` for slot 4
  (spell, scale, group, rand, recount, death).
- **Private session probe** (`.tools/review-native-ghost.c`, muted, in the actual app). It covers
  the zombie corridor with an explicit Ghost Card, selection and protection:
  - follow and attack updates, 210 ring updates on 1,680 real rows and 7 slot-4 attacks;
  - enemy damage, a kill and familiar EXP 0 to 4;
  - the sprite's drawn pixels;
  - Ghost to Sword to Ghost switches through the menu-close path;
  - at level 75, soul-steal attacks creating orbs;
  - deselection releasing every row.
  - Two mutants fail it: no slot-4 combat channel, and a box that outlives the one-update
    hitbox.

These checks use explicit fixtures. They are not evidence of acquisition, routes or console
parity.

Adapted source: `src/servant/tt_001/ghost.c`, the shared servant helpers it includes and
`src/dra/alu_anim.c` at the pinned reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
