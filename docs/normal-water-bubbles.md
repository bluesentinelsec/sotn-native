# Normal Alucard water bubbles

Standing and crouching Alucard now request original underwater breathing particles
through the actual shared factory and particle owners. These are separate from
[Wolf's breathing](player-water.md). They change no resources, relic ownership or
ordinary save data.

The original standing branch requires foot depth greater than48 and no raised
shield. It captures X±4/Y−16 before collision, retaining fractions and facing.
Crouch substate zero uses depth greater than24 and X±12/Y+2. Both share the original
96-update DRA counter. Standing with a raised shield holds the counter; eligible
crouching decrements it even with a shield. An expiration update only reaches zero;
creation waits for the next eligible update. Eligible dry updates still decrement.
Walking, jumping, other crouch substates, menus and spell freeze do not advance it.

The original request starts its cooldown even if optional allocation fails. Native
creation uses the real blueprint, shared slots and private particle profile; no
second pool or fictitious readiness is introduced. Primitive exhaustion retires
the optional child before randomness. A breathing request and walking puff can
coexist in one player update; the bubble request precedes the puff as in the
original. Freeze clears one-shot requests and room rebases move captured origins
without another simulation update.

The DRA counter persists through room entry and native stage/checkpoint/load
replacement. Candidate sessions restore it after bootstrap updates, preserving
its value without an extra decrement or copied one-shot request. Fresh processes
start at zero. Version-nine ordinary saves remain unchanged; this runtime counter
is not serialized.

Public authored tests cover stance/shield/depth boundaries, expiration, source
fractions/facing, shared-request ordering, real row exhaustion before RNG, snapshot
immutability, freeze and room rebasing. A private instruction comparison checks672
supplied contexts over80,640 original branch updates, including temporary position
restoration and ignored factory-allocation failure. Debug and ASan/UBSan traces
match byte for byte. Native room8 checks exercise both facings, real controllers,
factories and seven-row particles, menu holding and actual stage replacement.
Standing uses a fixed-floor pose fixture; crouching uses ordinary room collision.
Explicit positions, contact protection and a queued stage transfer are not an
uninterrupted-route or console replay, and no user save slots are written.
The actual CMake-built Debug app also exercises two standing and two crouching
casts, real particles and menu holding under muted LLDB, then exits normally.
That check uses an explicit fixed-floor pose fixture for both stances.

Water damage/death and broader Alucard/castle/console parity remain incomplete.
The existing US smoke birth path with status0x20000 remains explicitly unsupported;
this work preserves its guard rather than guessing an unwritten original value.
The authored shared-order test supplies a supported smoke context and does not
establish ordinary underwater walking-puff parity.

Provenance: `dra/71830.c:PlayerStepStand/PlayerStepCrouch`, pinned `sotn-decomp`
revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Private tables, artwork and comparison traces remain excluded from public files.
