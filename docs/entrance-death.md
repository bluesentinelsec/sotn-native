# Death's opening event

The native Entrance now runs the selected US Death event from the user's private
NO3 script. It controls Alucard's approach, starts the original conversation,
advances Death/scythe animations, displays private portraits and glyphs, plays
five original XA speech clips, takes the six targeted equipment items, presents
the departing icons, changes to Dracula's Castle music, fades the background,
and returns player control. Original event flag 53 prevents repeating the scene.
The first hallway's flag-55 trigger now enables floor-zombie spawning and changes
its music; lighting/palette effects are still incomplete.

Use the project's executable through LLDB:

```sh
make debug
lldb -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native --game-data local/game-data/us --entrance --entrance-room 11
```

Enter `run`. If a crash stops the process, use `thread backtrace all`.
This explicit room start now uses the original [opening equipment](opening-equipment.md)
with the existing 70-HP/base-stats development profile.
It is not the original new game or a prologue-derived starting loadout. Death
removes one copy of each original targeted item, preferring an equipped copy over
inventory; he leaves unrelated equipment and resources intact. An item need not
be owned for its original cutscene icon to appear. Ordinary starting sword/shield behavior is integrated; command attacks and
shield-rod spells remain incomplete.

The bounded script service reads original event/flag/sound/text/portrait commands
without executing machine code. It validates offsets, flags, slots, text capacity,
and instruction budgets. Actors refer to related slots by generation. Closing or
changing rooms discards transient event state; supported permanent flags belong
to the world/save owner. Full Time Attack records and restart-mid-event state
are not implemented.

The original five dialogue clips use the existing stereo 37.8-kHz XA decoder.
Finite clips now flush SDL's resampling tail after their declared source duration;
a clip ending exactly on a sector boundary no longer requests unrelated data.
Ambience uses the original channels 20/21 and releases with the original rate
command at the transition. The sound-mode mapping now follows the original
0=pooled 12..19, 1=22/23, 2=release 22/23, 3=20/21 assignment. Full SPU/CD/reverb
and hardware scheduling parity remain unproven.

The dialogue window is a native framebuffer adaptation using original private
font/portrait artwork and script positions. Full original window animation,
nameplates, sky swirl/lightning/palette changes, the special Alucard theft pose,
Death's entrance glow, icon collapse deformation, and exact GPU priority/blending
remain presentation/parity work. Do not call the opening or Entrance fully authentic.

Public synthetic tests cover script/event execution and budgets/bounds, speech
waits, once-only state, manager fade/control release, stale scythe ownership,
afterimages, targeted theft, ambience release, finite-XA tails, and presentation
bounds. Private LLDB/ASan checks complete the conversation with all five clips,
verify theft of privately granted inventory items and preservation of Gladius,
and verify returned control/music. These are explicit room starts, not a whole
castle replay or a user playtest.

Adapted AGPL-3.0-or-later behavior: `src/st/no3/e_death.c`, `cutscene.c`, shared
cutscene event/window helpers, `e_background_lightning.h`, `dra/5F60C.c` and
`dra/sound.c` at reconstructed commit
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. XA decoding retains the existing
MPL-2.0 PSY-Z provenance. No dialogue strings, scripts, artwork, audio, palettes,
or original runtime arrays are embedded or published.
