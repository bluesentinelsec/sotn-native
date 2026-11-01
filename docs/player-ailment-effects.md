# Selected poison and curse presentation

Actual surviving normal-player poison/curse contacts now request their original
shared presentation. Curse takes priority over poison: it sets word1/palette8165,
requests text2 then Blink17 and queues voice6F2. Poison sets word0/palette8164,
requests Blink16 then text1 and queues the same voice. Old curse/poison restoration
and hurt motion factories remain earlier in the original order. Axe Armor retains
its separate early return; it receives no new ailment from this tail.

The existing entity37 owner now accepts only its selected full child parameters
0000/0100/0200. It still waits for current or prior grounding before reserving its
actual eighteen stage rows. Row0 is the private text GT4, row1 stays hidden, and
rows2..17 are the original sixteen one-pixel grey tiles. Metadata and artwork stay
private. No additional entity, primitive or effect pool exists.

Immunity0 preserves its original phase5 retirement. Poison1/curse2 return from the
right edge to the original packed target, whose low word retains the initial
half-width. They queue one actual guard tink when reaching it, hold for96 updates,
then accelerate left and retire offscreen. Preserve both distinct sine-angle cap
orders, signed word/fixed wrap, the12-update trail and the final Y increment on a
tile's retirement update. A retired tile loses its camera flag; a later reuse does
not manufacture that flag again.

Ordinary scrolling changes only tile packets with original draw flag2. The text
extension coordinates and its flag100 GT4 remain fixed on screen. Room entry has
a different original packet pass: it shifts the flag100 GT4 once without advancing
or changing the text controller/packed target. The following ordinary update
rebuilds the quad from its retained extension coordinates. These rules also correct
the earlier immunity-only implementation's unconditional coordinate translation.

Text and Blink run through their existing actual child order, row leases,
rendering, audio requests and freeze flags. Menus and repeated draws hold them;
selected spirit freeze advances them while player ailment words hold, and the
release-only frame holds the runtime. Invalid metadata or a late owner failure
preserves runtime/resources/leases. Optional factory or row exhaustion omits only
the visual. No ailment counters, text cursors or row leases enter version-nine
saves, and ordinary resources/equipment/relic grants retain their existing defaults.

Authored tests cover ordered ailment factory calls, curse priority, real shared
Blink/text leases, delayed grounding, full text lifetime, packed fractions, sound,
no-tick camera/room passes, frozen advancement, signed tile wrap, metadata rollback,
optional row exhaustion and retirement. Private comparisons match1,296 original
text contexts/165,024 updates for parameters0/1/2, including all sixteen tile
packets, SDK trig, targets, sounds, allocation failure and camera/room packet rules.
Another1,536 incoming effect-mask contexts match original ailment priority,
counters/palettes, ordered factory sources and voice. Allocation/destruction,
factory/audio, debug print and CON6 duration services have explicit adapters;
these instruction checks are separate from native scene/console evidence.

Muted native checks retain default70HP/20MP. A spawned/positioned actual Library
seed queues poison, producing70-to42 HP and duration3,999; a placed/positioned
Ectoplasm queues curse, producing70-to64 HP and duration1,000. Both create real
text and Blink rows. Their text contributes159/147 pixels and their trail
contributes11 pixels, reaches the packed target, queues one tink and retires.
Six menu updates hold all sixteen words and the runtime; actual Summon Spirit
advances effects through33 frozen updates and preserves the separate release hold.
Its real enemy kill/level-up HP/MP changes remain intact. Repeated draws do not
advance owners. The actual CMake app independently checks the placed curse
contact,147 contributed text pixels, real trail/target/retirement without replay
and normal SDL quit in Debug and ASan/UBSan. Explicit Library arrivals, seed spawn/actor positioning and later
protection fixtures are separate from acquisition, uninterrupted traversal and
console comparisons. Actual-app/sanitizer/platform results are recorded in the PR.

Selected [cut feedback](player-cut-effects.md) now owns its actual children.
Other elemental children, rare stone/grab/death/fade branches, other text
parameters, equipment/familiars, full shared contention and integrated acceptance
remain open. This batch adds no dispatched player state and does not complete
[Alucard milestone1](alucard-status.md), castle progression or console parity.

Provenance: `dra/71830.c:AlucardHandleDamage`, `7879C.c:EntityGuardText` and
`EntityPlayerBlinkWhite`, and `5087C.c` ordinary camera/room-entry packet passes;
pinned AGPL-3.0-or-later sotn-decomp7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
