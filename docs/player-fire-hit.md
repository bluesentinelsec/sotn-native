# Selected surviving fire-hit ownership

Surviving normal-player fire contacts now request sound690, Blink45, factory17
parameter1 and factory18 in original order, with palette8160/timer2 sixteen.
Existing curse/poison priority stays earlier; fire excludes the subsequent thunder,
ice, holy/dark/cut tail. Old ailment/motion roots remain earlier still. Axe Armor
and lethal/death dispatch keep their separate paths. This does not complete fire
death or the alternate effect10000 path through factory19.

Factory17 supplies its actual six high1 children with twelve-update cadence.
Factory18 supplies its actual delayed low-parameter stream, sixteen initial updates
and three-update cadence, with128 as an upper bound. Both use original origin7:
leaving actual hurt state10 cancels further births. Existing children finish their
own private timelines. Actual shared root/child exhaustion omits optional effects;
there is no replacement cooldown, pool or unrelated actor replacement.

Entity16 retains source fractions/facing and reads the live pre-camera player high
words, raw hitbox Y and actual velocity. Three birth RNG calls choose priority
above/below the player and X/Y offsets. Captured velocity uses signed arithmetic
quartering, with retained upward half-pixel motion. Birth has no movement. Later
callbacks shrink both scale words, move with signed fixed-point wrap, set original
blend phases and test the prior negative animation timer after final motion.
Private animation follows callback/offscreen culling. Camera translation follows
those operations; room entry rebases without a tick. No ordinary player movement
or stage shove is substituted for camera-locked entity displacement.

High1 and selected low variants use actual private bank2 animations/scales; every
fourth low child uses bank5/palette8170. That branch's zero scale words still wrap
when decremented, while its disabled scale flags keep unit geometry. Actual SDK
scale rounding, original mirrored UV endpoints, multipart order, private palettes
and raw texture blending render through the shared graphics owner. No invented
opacity fade or source-art modification is introduced. Supplied high2 component
comparisons do not establish a native producer for that variant.

A non-smoke even-parameter child requests actual factory4/profile4 at pose8 with
one remaining animation update. The factory retains its real parent slot and
generation; its descendant is born on the next factory pass. After real allocation
of ten tiles plus the sentinel row, the child reads that actual parent's retained
velocity. It never substitutes the current player velocity. The selected parent
remains alive at this boundary; missing/mismatched ownership fails explicitly.
Other root roles clear this metadata on reuse.

Profile4 consumes three RNG calls per particle only after successful row allocation.
It starts from whole-word origins, retains the parent's X velocity, but moves only
Y in original behavior3. Delay is index-times-two plus15, with signed decrement
and hiding below zero. Colors do not fade. Lifetime31 takes two decrements per
update, leaving fifteen motion updates before retirement on the sixteenth. The
sentinel shares the actual lease and retires with the child. Optional row failure
retires before RNG or parent reads. Late failures undo runtime, parent requests,
row plans and RNG together.

Authored tests cover real parent identity/velocity versus changed player velocity,
shared rows, unrelated lease retention, finite animation/blend/request timing,
hurt-exit cancellation, child-slot/row exhaustion before RNG, signed wrap,
no-tick room rebases and active child/particle/root frozen holds. A lower-slot
particle birth followed by an old fire child's malformed animation fails atomically.
Combined incoming effects preserve six ordered hurt requests after old ailments.

Private comparisons match3,408 parameter/seed/wrap/velocity contexts and91,344
original controller/UpdateAnim updates, including888 correctly phased particle
requests. Profile4 matches24 parent/null/word-wrap/allocation contexts,216 updates
and1,920 particle records. Original factory scheduling matches64 contexts/4,316
records/1,173 allocations; Blink45 matches144 normal/Bat/rotation contexts and9,936
complete snapshots;256 incoming effect masks preserve original tail priority and
sound/factory order. Another368 native private frame/facing/blend/scale contexts
match380 GT4s through the original full entity renderer. Allocation/destruction/RNG,
factory/audio, SDK CLZ and explicit engine GPU/identity-CLUT/camera/room setup use
service adapters. Private input tables, artwork and raw traces remain local.

Muted native checks retain default70HP/20MP and walk/settle at an actual Laboratory
wall/floor. Supplied normal/heavy raw fire contacts produce50HP, palette16, three
birth RNG calls, both sprite banks, Blink45/eight rows and actual parent-bound
profile4/eleven rows. Normal/heavy contexts contribute409/392 sprite pixels,
755/743 Blink pixels and10 particle pixels. Live draws and actual menus hold owners/all sixteen player
words. Hurt exit cancels both roots and finite children retire without replay.
A real Summon sequence checks33 frozen updates plus its release frame after fire
retirement; active fire/particle frozen holds are separate authored evidence.
The actual CMake app separately checks both banks/blend phases, contributed pixels,
real descendants, cancellation/retirement and normal SDL quit. Raw contact/kind,
protection and wall fixtures are not ordinary fire enemy producers, acquisition,
uninterrupted routes or console parity. The app contributes409 sprite/755 Blink/10
particle pixels in Debug and ASan, then exits through SDL. All206 tests pass in
Debug/Release/ASan-UBSan;34 Python tests, policy897, formatting, static SDL linkage
and ZIP packaging pass. Five Debug/sanitizer controller/particle/Blink/producer/
factory trace pairs are byte-identical. Platform results are recorded with the PR.

Thunder, ice, remaining stone/grab/death/fade, equipment/familiars, full shared
contention and integrated acceptance remain unfinished. No player dispatch state,
resource grant, ordinary-savev9 field or user save slot changes. The
[Alucard milestone1 gates](alucard-status.md) remain open.

Provenance: `dra/71830.c:AlucardHandleDamage`,
`7E4BC.c:func_8011EDA8`/`func_8011E4BC`, `alu_anim.c:UpdateAnim`,
`7879C.c:CreateEntFactoryFromEntity`/`EntityFactory`/`EntityPlayerBlinkWhite`/
`UpdatePlayerEntities`, `4B758.c:RenderEntities`/`func_800EB758` and engine passes
in `5087C.c`, sotn-decomp commit `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
