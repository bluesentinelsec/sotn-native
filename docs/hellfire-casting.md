# Hellfire casting controller

The core reconstructs the selected original PlayerStepHellfire controller. This
component enables no native Hellfire controls. It requires actual factory outcomes,
player collision/animation snapshots and the shared beam signal; its ordered calls
are requests for the native owner, not completed effects or readiness.

Phase zero requests the real root factory before animation one. The owner must
perform that allocation before supplying its outcome and acknowledge the emitted
record without allocating twice. Failure invokes the actual normal helper, then
still selects animation one and increments its resulting substate, as the original
does. The retained shield substate comes from that helper's real owner.

Hidden movement uses original right-before-left direction priority, movement lock,
preceding side/center floor masks and strict screen-edge comparisons. Signal two
stops movement and selects reappearance. Signal three selects attack recovery;
Down cancels only at that boundary. Pose ten/duration one requests timer twelve
and the expanding-circle factory in that order. Negative duration or FFFF invokes
the normal helper, recovery animation and original status-flash restoration. An
allocation failure combined with FFFF preserves both normal-helper calls.

Actual MP/learning, state entry/invincibility, shield removal, root/child generation,
gradients/palettes, spell attacks, child animation, drawing, room rebases and native
combat remain separate ownership work. In particular, finishing requests the real
status-flash helper; it does not invent a palette reset. Ordinary starts and v9
saves remain unchanged. Selected Dark/Summon/Tetra remain playable; Hellfire stays
disabled until the full native path connects.

Public tests cover the lifecycle, signal waits, strict edge/floor movement, both
facings, opposing input, movement lock, Down cancellation, timer/factory ordering,
ordinary and shielded fallback, double fallback and invalid-state rollback.
Private instruction comparisons match 6,000 supplied phase/facing/collision/signal/
factory-outcome/helper/animation contexts. The original controller, direction
helper and SetSpeedX execute; factory, normal, animation and restoration calls
use explicit adapters. These are scoped component comparisons, not actual factory
allocation, player movement/animation, a native spell, Alucard/castle or console
parity. Private inputs and traces stay local.

Provenance: selected `dra/71830.c:PlayerStepHellfire` and
`dra/6DF70.c:CheckMoveDirection`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
See [beam lifecycle](hellfire-beam.md), [supporting effects](hellfire-effects.md)
and [projectile motion](hellfire-fireballs.md).
