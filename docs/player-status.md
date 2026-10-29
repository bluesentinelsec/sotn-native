# Selected Alucard status reconstruction

Native player snapshots now rebuild the supported original status word from the
current movement state and owned timers. Standing, walking, crouching and airborne
states retain their original normal-body bits. Walking includes the capture-clear
bit; seated standing includes the afterimage suppression bit. Hit, stone and death
retain their distinct normal-body, recovery, death and afterimage flags. Existing
Bat/Mist/Wolf, return, high-jump, stuck, grab and scripted states keep their selected
original masks. Crouch substate two and death substate0x80 remain distinct.

Owned invincibility and subweapon-use timers add their own original bits; use time
never becomes invincibility. Poison/curse and the existing bell adapter retain their
separate flags. The snapshot does not carry stale form/status bits from earlier
states, mutate movement, advance timers or allocate resources. The collision
owner still maintains its existing selected form word at its original update point.

The native MP owner now sees the original recovery-freeze bit during physical hit
and stone states. Normal recovery resumes regeneration on its existing absolute
clock. The renderer sees walking's actual capture-clear bit and clears the owned
63x63 transformation target. The afterimage owner consumes the same reconstructed
NO_AFTERIMAGE bit before animation/collision instead of maintaining a second
selected-state list. No extra simulation or display ticks are introduced.

Public authored tests cover all supported masks and selected substates, combined
ailment/use/invincibility timers, stale-bit rejection and immutable snapshots.
Private comparisons match 2,592 original status-construction instruction contexts.
They adapt invincibility/afterimage helper calls at explicit boundaries and compare
status assembly; they do not prove those helpers or a complete original player frame.
Debug and ASan traces match. Original inputs and traces remain private.

Muted native Step/Draw and actual CMake-executable checks preserve standing's
retained capture, then verify walking clears all 3,969 owned pixels. A controlled
one-MP cost and one-point physical hit verify no recharge at tick64, followed by
normal recharge at tick128. They use the ordinary development character/enemies,
with no relic, protection, position or stat grants, and write no saves. Debug and
ASan/UBSan pass. These explicit fixtures are not ordinary acquisition, uninterrupted
route, full-frame or console evidence.

Full status-driven palettes, invincibility-helper timing, all original transient
timers, water, spells, equipment/familiars and shared GPU/factory parity remain
incomplete. Ordinary starts and version-nine saves remain unchanged.

Provenance: selected `dra/692E8.c:EntityAlucard` status reconstruction at pinned
sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
