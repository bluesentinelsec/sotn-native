# Selected native MainGame tail

Startup and gameplay now execute the original post-render MainGame tail once per
successful native engine frame. The actual four signed HMS/frame words retain
[playtime ownership](player-playtime-badelaire.md). The original map-cursor word
advances by the normalized vertical count, and the existing native libc cursor
advances through one trailing `rand()` call. Inventory/map pause holds only HMS;
it does not suppress either of those other operations. Even a normalized count
of zero still consumes that one call.

## Native ownership and ordering

`SotnMainLoopTail` is a working snapshot of existing owners, not another timer or
RNG pool. The pure component normalizes the supplied signed SDK count, wraps the
map word, applies the original single-carry status clock, then advances the existing
native libc adapter. Gameplay commits time and its real character-owned libc cursor
atomically before publishing the map word. Failed updates and pending-stage catch-up
calls do not run this tail. Existing earlier player/stage rollback boundaries remain
separate; this does not establish whole-pass atomicity.

The map packet samples the cursor before the original tail increments it. Native
rendering keeps that packet-phase snapshot separately from the live word. Every
ordinary, frozen, release, FFF, menu, map and Game Over update advances the live
cursor. Opening the map no longer begins a separate map-only clock. Repeated draws
reuse the same sampled phase and never consume RNG or advance any clock. Signed
original words retain their unsigned32 bit patterns through wrap.

The resident/title owner runs the tail after its actual title/menu, primitive,
pixel and audio work. This allows title lightning to observe the earlier real
libc history while held input, row failure and repeated draws preserve their
existing boundaries. The native filesystem Continue adapter still supplies its
explicit menu behavior; this does not enable the original SEL menu or New Game.
Cold Continue carries the actual map word into the candidate with no extra tick.
Connected travel, in-session Continue and the native development restart retain
both the live map word and the last prepared packet phase. Saved loads restore
HMS/RNG from their existing snapshot; the native development restart resets its
character/time without resetting the global map word. Neither transient word is serialized.

Native fixed60 frames supply SDK count256. The component preserves source signed
normalization, zero/negative counts, single carries and wrapping words for supplied
contexts; it does not implement console raster stalls or the hardware vertical
counter. Original MainGame reaches BIOS A0/2F for `rand`. This binding uses the
existing native libc LCG/initial cursor adapter, not embedded BIOS code or a BIOS
boot-state reconstruction. Native stage/combat RNG cursors and the v9 saved-RNG
restore policy retain their existing adapters: startup begins at1, native
development character creation keeps its existing zero cursor, and saved loads
restore the v9 character cursor. Cold Continue still restores the
saved character cursor and consumes actual AluInit initialization calls; it does
not claim continuity of every original BIOS RNG consumer across the save menu.

No resources, equipment, abilities or saves are granted or added. Save layout
stays v9. Full SDK timing, global RNG sharing, native engine/fade/loading ordering,
remaining Alucard owners and all six [milestone gates](alucard-status.md) remain
open, alongside [cold New Game/prologue](cold-start.md).

## Evidence

Public `main-loop-tail` cases cover paused status with advancing map/RNG, word
wrap, signed normalization, zero-count RNG consumption, null failure and atomic
character clock/cursor binding without changing ordinary resources or stats.
The existing status-clock and actual hand/property tests retain their scope.

Private comparisons execute367,416 complete original MainGame tail contexts,
including the actual BIOS `rand` trampoline and the order of original map/status
writes before its exactly-one call. Counts, flags, four words and input cursors
are supplied fixtures. The BIOS operation is an explicit native libc adapter;
this is not full BIOS/SDK or console evidence. Debug and ASan traces agree byte
for byte. Native protocols separately observe actual map packet phases, owner
commit boundaries, title/Continue, connected travel, spell/Fairy freezes, death,
shared hands/forms and selected failures. Logs and reference traces stay private.

All234 public groups pass in Debug, Release and macOS ASan/UBSan. Twenty-four
sequential native protocols pass across Debug and ASan, with explicit pass, normal
inferior exit and no sanitizer diagnostics. The added repeated failed-hour test
also passes focused checks in all three configurations. Supplied fixtures and
adapters retain the boundaries above; these checks do not close a milestone gate.

Source: `dra/42398.c:MainGame`, `dra/5087C.c:RenderMap` and
`st/sel/34048.c` at AGPL-3.0-or-later sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. Private programs, BIOS inputs and game
artwork remain external; no original binary/table is embedded.
