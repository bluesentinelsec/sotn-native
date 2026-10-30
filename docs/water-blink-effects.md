# Water-pain and death flashes

Selected BlinkWhite parameters4E and59 now run through the existing shared effect
runtime. Their eight GT4 rows, original private profiles, transform geometry and
afterimage resets use the same real owners as the other selected player flashes.
The native scene supplies actual waist depth, found-and-active Holy Symbol and
current animation ID. This does not yet enable native water damage or water death.

Water pain's profile holds while waist depth is nonzero and Holy Symbol is inactive.
Leaving water or enabling that relic starts the fade. The death profile instead
holds on animationC0; it releases on C1, preserving the original condition even
though water death itself uses C1. Both retain original ramp, repeated hold-counter
assignment/decrement, fan colors, rotation and finishing behavior. Neither provides
a transformation signal. Existing Bat/Wolf locks and Mist incompatibility still
apply. Real row exhaustion retains the uninitialized US child for retry; repeated
full-parameter requests suppress duplicates. Rebases translate packets without a
simulation update. Menus and draws never advance the owner.

Public authored checks cover hold/release conditions, pending row exhaustion,
duplicate suppression, actual shared-row plans, no-tick rebasing, retirement,
unchanged HP/RNG and malformed-depth rollback. Private Debug and ASan/UBSan traces
match byte for byte across288 supplied normal/Bat image contexts and25,728 complete
controller/shared-phase/eight-quad snapshots. The original-instruction comparison
uses actual image lookup, transforms, SDK geometry, private profiles/colors and
afterimage helper, with allocation/destruction/CLZ and Holy Symbol query adapters.
Supplied player snapshots are not a complete original engine or console replay.

Muted native Step/Draw checks use actual Entrance room8, stage water, shared
factories/primitive commits, uploaded artwork and native drawing. They exercise
1,272 GT4 draw packets,49 held pain updates, duplicate requests, menu holding
and actual world Holy/animation retirement. The checks use explicit position,
fixed-floor pose, contact protection and supplied factory/relic fixtures; they do
not establish water HP loss, actual dying-player capture, ordinary acquisition,
whole-route or console parity. No user save slots are written.

The [water damage/death controllers](water-damage-death.md) still need actual
equipment, HP, remaining birth effects, capture and common death ownership before
live use. Ordinary starts, grants and version-nine saves remain unchanged. Alucard,
the castle, full shared effects and console parity remain incomplete.

Provenance: selected `dra/7879C.c:EntityPlayerBlinkWhite`, pinned `sotn-decomp`
revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Original profiles, artwork and comparison traces remain private.
