# Fairy control and native player inputs

The original US Fairy now has bounded component callbacks for root/wings
initialization, normal following, ability selection, Hammer, Uncurse, Antivenom,
elemental resists, potions and Life Apple. Native player snapshots also supply the
actual transient damage-effects word and pre-control step/substep. These are
dependencies for the familiar owner. Fairy activation, real slots4..7, item use,
rescue state17 and progression remain unavailable; ordinary starts and savev9 are
unchanged. All six [Alucard completion gates](alucard-status.md) remain open.

Initialization distinguishes engine0/2, menu switch1 and same-servant3. Same3
requests the original event-queue reset while retaining both cursors. Other modes
preserve full player fractions, signed fixed-point wrap, save-room side rules and
the original priority offsets. The component does not upload graphics, release
old rows or bind native slots. Those operations require the actual lifecycle owner
and the existing [familiar resource services](familiar-foundation.md).

Following retains the original pre-initialization wave calculation, private SDK
sine/cosine/angle/square-root interpretation, signed-word winding, distance bands,
quarter-distance catch-up, facing and animation thresholds. The original movement
gate uses actual velocity, collision and pre-control state history. Wings copy the
real parent's position/facing and resolve their private animation/priority map;
their parent union is never treated as root ability words or an emulated pointer.
Callers own clocks, slot generations and no-tick rebases. Extreme square-root
inputs outside the existing SDK service's nonnegative signed domain fail explicitly.

Selection preserves death before even a negative medicine timer, the exact libc
RNG calls, private level bands, Hammer/curse/poison/resist/potion priority, elemental
delay/clear history, five HP-loss entries and the original potion-luck behavior.
The late quarter-HP/history tests may overwrite an earlier potion urgency. Private
tables stay private. Invalid history, mappings and late animation failures leave
the entity, history, RNG and output unchanged.

Item callbacks retain their approach and pose gates, potion fallback, actual
unequipped-count identity, factory55 parameters, local event requests and VAB sound
selection. Only the original callbacks that check active child39 apply that gate;
Hammer omits it. Optional factory/event exhaustion still spends the medicine.
Requests are not completed inventory writes, effects or healing, and these
callbacks must not inherit manual-hand Duplicator semantics.

Life Apple retains the greater-than24 wait, bag check and wait for scene0 before
consumption/sceneFFF. Its initial25-frame counter carries through approach/casting;
subsequent phases request the item event, Life Apple effect, conditional child29
flash clear, real8..63 cleanup, full HP/MP restoration and later scene release.
Resource restoration and release are distinct phases. The91-frame counter is
retained on release, so the following greater-than60 return happens immediately.
No-apple animation/sound then hold remain separate. These requests supply neither
completed cleanup nor revival. Native FFF must run the full player and only
familiar4..7, followed by the frozen stage dispatcher; it must hold shared8..63.
The existing Spirit freeze3 path cannot substitute for this engine path.

The native player clears its actual `unk18` equivalent at every successful full
player prefix, including dead updates. Resolved damage responses1/2/3/7/8 write
their effects. Immunity, absorption, ordinary water and lethal4/6 leave it zero;
stone entry7 still writes when its HP result is zero. Freeze, menu, draw and room
rebases retain the last committed value. A failed update retains the prior value
and queued contact. Separate control-step/substep snapshots capture the original
`prev_step/prev_step_s` boundary after contact/form selection and before the state
callback. They do not replace the movement owner's earlier transition history.

Public authored regressions cover selection priority, potion urgency overwrite,
one-time fallback flags, initialization/fractional wrap, wing-parent failure,
scene/item/resource phase boundaries, native damage writes/exclusions/holds and
late failure rollback. Private original-instruction checks compare28,707 complete
selected callbacks plus192 initialization,14,400 wing,10,000 signed-angle and512
movement contexts. Original animation, movement, local event creation and SDK math
execute; explicit adapters supply stats/buff snapshots, BIOS RNG, GTE leading-zero
registers, room events, factory/audio/shake requests and initialization uploads.
Wing comparisons cover its actual pose/flags/priority, excluding the original
parent-pointer union from root-extension comparisons.

Actual native checks separately exercise the player input providers with private
US resources. Explicit raw contacts, attributes and HP1 are controlled fixtures,
not familiar activation, item acquisition, ordinary encounters, routes or console
evidence. Eleven native contexts pass in both Debug and ASan/UBSan, including
menu/draw holds and a real standing-to-walk control snapshot. Both actual
CMake-built apps also pass the owned-medicine regression through ordinary use,
real shared presentation/resources, holds and retirement. All217 authored tests
pass in Debug, Release and ASan/UBSan; Python tests, repository policy, formatting,
static SDL linkage and Release packaging pass. Private component traces match
between Debug and ASan. These checks do not establish full familiar availability.

Remaining owners include intro/shoulder/hint behavior and real room events/CD/XA
dialogue, native slots/generations/rendering/shared leases, item/event primitives,
FFF/player17/recovery/cleanup, selection and mutual exclusion, and saved familiar
progression. No new entity/primitive pool or default grants are introduced.

Provenance: `servant/tt_002/faerie.c`, common servant movement helpers and
`dra/692E8.c:EntityAlucard`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
