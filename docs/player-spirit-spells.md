# Selected playable Summon Spirit and Tetra Spirit

The native checkpoint now connects both spirit commands to original player cast
states, private MP costs, learned progress, actual shared factories/children,
framebuffer capture, freeze/release, artwork and native enemy combat. Selected
Dark Metamorphosis also remains playable. Hellfire, Soul Steal, Sword Brothers,
other equipment/familiars/water and full Alucard/castle/game parity remain incomplete.

Summon Spirit uses Back, Up, Down, then Down plus either hand attack. Tetra Spirit
holds Up for the original 32-tick threshold, then Up-forward, Forward, Down-forward,
Down, then Down plus either hand attack. Relative directions retain each command's
captured facing. Private US costs are 5 and 20 MP. Exact-cost casts work; insufficient
MP produces no spirit cast, charge or learned progress. Successful casts learn the
original spell once and persist that ordinary progress through the existing v9
format. No default relics, resources, equipment or spell-learning grants are added.

The actual portal child controls scene freeze. Its birth frame still completes
ordinary player/stage simulation. At that frame's end the engine clears the real
afterimages and queues sound-script pause after its ordinary sound requests.
Subsequent frozen frames advance the engine clock, selected cast animation,
flag-permitted shared children, original eligible/new stage actors, palette jobs,
shake and audio. Player movement/collision, game clock, MP recovery, status/use/
invincibility timers, stage hit detection/cooldowns, ordinary projectile updates,
saves and menus stay held. Current physical buttons still update their adapters.
The retiring portal completes its final frame under frozen rules. The next frame
only applies original protection and script resume; normal updates resume one
frame later. Pending blood healing is retained, never replayed during freeze.

When the nine real portal rows become available, its actual capture intent copies
exactly 63×63 RGB555 words from the preceding ordered game frame to the shared target
at 512,448. This happens once during simulation, before the current frame replaces
the source. Skipped presentation flushes the previous frame first; repeated draws
never repeat the copy. Adjacent target texels remain intact. The common Wolf
capture cache uses an actual player-effect-update clock that advances through
spell freeze and stays held through menus. A frozen game clock cannot restore a
stale target over a newly captured portal, and menu redraws cannot advance effects.
Off-frame sources fail clearly because surrounding PS1 framebuffer VRAM is not
integrated; no clipped or placeholder capture is substituted.

The portal draws its actual black ninth-row rectangle and eight raw textured
wedges. Spirits use private bank-nine body artwork/palette, original rotation and
quarter blending, with eight real retained trails. Selected BlinkWhite 0x67 uses the
actual uploaded player image. Child masks, real row leases, private attacks,
original target marks/cursor and group cooldowns feed native enemy damage/death/EXP.
New target marks are acknowledged after their generation-checked stage commit.
The raw retained target slot keeps original nonzero-replacement/zero-identity
semantics. The engine preserves its circular cursor across session replacement;
it does not serialize that global as ordinary save progress. Fresh stage/load
sessions clear transient cast, freeze, factory and effect owners.

Public authored tests cover exact63×63 copies, source/target bounds and ownership
failures, untouched neighboring texels, replacement captures, and bounded new draw
commands. Existing command/cast/factory/freeze/target/group/combat tests continue to
cover their focused boundaries. Public CI contains no original artwork or tables.

Muted private native checks cast both spells in both facings using ordinary MP,
INT and enemies, with explicit x124/contact protection. Each observes 33 frozen
updates, verifies unchanged player/character/game/actor clocks and timers, compares
every captured word to the real preceding frame, renders actual bodies/trails and
Blink, and checks native enemy hits. Repeated draws remain stable; skipped draws
still preserve capture. Separate checks exercise four Summons with overlapping
real generations, full-parameter flash deduplication, insufficient Tetra MP,
in-memory v9 learning/load/reset and actual native session replacement preserving
a supplied cursor 113/system clock. Save-room/cursor transfers are explicit fixtures;
no ordinary save slot is written. Existing owned-Wolf menu/return checks verify
that pause still retains its body/resources/runtime.

Actual CMake executable runs under muted LLDB also execute both commands, real
children, frozen clocks, separate release and enemy deaths, then quit through an
ordinary SDL event. Debug and ASan/UBSan runs are checked separately. These
positioned/protected checks are not uninterrupted routes or unmodified encounters.
The prior [shared-runtime instruction comparisons](shared-spirit-runtime.md),
[cast/freeze](spirit-casting.md), [stage dispatch](stage-spell-freeze.md) and
[sound-script](spell-sound-freeze.md) comparisons retain their separate scopes.
Full engine/camera timing, global primitive/equipment/familiar contention, OT/raster,
HUD interpolation, audio/SPU and console parity remain unproven. Do not describe
these selected playable spells as complete Alucard or a complete game.

Provenance: native adaptation of selected `dra/5087C.c` engine boundaries,
`6DF70.c` freeze animation/protection, `84B88.c` spirit/portal, `7879C.c` factories/
Blink and `sound.c` script commands, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Original artwork, tables, executable comparisons and runtime traces stay private.
