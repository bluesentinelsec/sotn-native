# Olrox articulated movement components

The core now reconstructs the original monster's joint approaches, paired-leg
pose controllers, fixed-point hip/knee/foot anchoring and flat-arena floor contact.
These are components for the upcoming articulated body. They do not register or
advance the monster actor, render its parts or establish a playable second form.

Two owned leg records replace the original use of primitive packets as mutable
physics metadata. Each leg retains separate upper/lower angles and velocities,
phase, contact, knee/foot positions and horizontal distance. Its counterpart is
an explicit array role. One body generation owns the service; foot anchoring
requires the body's original owned children. State changes clear leg phases
while retaining joint angles, velocities and the body's animation cursor.

Preserve strict original arrival comparisons: landing exactly on a target may
report completion on the following tick. Smoothed approaches retain signed word
velocity and original acceleration. Joint routines preserve signed word wrap.
Forward/backward support motion updates body tilt independently of foot phases.
Grounded anchoring derives the hip from the support foot, while the airborne
solution derives both feet from the body's retained origin. Their original
four-pixel difference is intentional parity behavior. The floor is the original
arena-specific plane; contact at its exact boundary remains true with zero shift.

## Validation and scope

Public authored tests cover strict arrival, smoothing, all thirteen pose
controllers, retained state, exact straight-leg geometry, the four-pixel origin
difference, floor contact, malformed selectors/math input and missing/reused owners.
Private Debug and macOS sanitizer traces match 41,120 original instruction
snapshots over nineteen helper entrypoints, with both leg roles/facings, camera
origins, SDK lookup geometry, pose phases and completion results. Only debug
printing is isolated; original helper and SDK instructions execute. Seeded foot/
body fixtures supply initial component state. This is not actor integration,
ordinary boss traversal or console parity evidence.

The selected [native encounter](olrox-encounter.md) now advances these helpers with
real body, parts, morph and attack/death/reward controllers. Connected BO0/NO2 travel,
full presentation/parity, castle and Alucard behavior remain incomplete.

Provenance: reconstructed selected BO0 `313A8.c` routines
`func_us_801B1590` through `func_us_801B24CC` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
All original SDK tables remain private inputs; authoring disassembly/generated
analysis and private trace fixtures are not shipped.
