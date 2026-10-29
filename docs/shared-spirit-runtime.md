# Shared spirit factories, targets and combat

The selected shared player runtime now owns Summon Spirit/Tetra Spirit's actual
factories and children, private animation/attacks and real stage primitive leases.
It also supplies a native stage-target snapshot and damage adapter. The
[selected playable integration](player-spirit-spells.md) now binds actual capture,
presentation, freeze/release, sound commands and progress learning. These shared
interfaces alone do not make Alucard, the castle or the game complete.

The original root factory117 retains its full parameter. Its real child63 requests
freeze3 and flash13 at birth, waits ten updates, then reserves nine actual rows
before requesting the ordered63×63 framebuffer copy to512,448. The owner must
fulfil that intent; no timer manufactures capture readiness. Expansion requests
actual factories116/118 and the full parameter0x6700 BlinkWhite. Factories created
during the child pass first update on the following frame. Descendants inherit
the original initialized root flags, including the frozen-update permission.

The shared dispatcher snapshots the incoming freeze request before any factory or
child changes it. Frozen updates preserve unflagged factories/children, while
initialized original flag0x10000 children continue. Existing Fire/Wing/Echo attack
ownership remains represented while their controllers are held; the frozen stage
path must perform no hit detection. Global flash decrement/drawing follows the
child pass, including a newly born portal or Dark flash. Wolf's selected body does
not advance in this path. Full cross-ability frozen contention remains unproven.

Each actual spirit61 reserves sixteen shared rows and runs private spell/INT/RNG/
Mojo Mail initialization once. Lowest-use groups3..6 belong to original slots17..31;
groups7..10 belong to32..47. Real group counts survive child retirement and Wolf's
forced replacement, including a spirit retained in slot31. Animation advances
**after** controller/trail capture; the birth sprite zero reads the bounded private
adjacent UV bytes. Four Tetra children retain parameters0..3 and the actual
blueprint's spell-one initialization. The packed high parameter still selects the
original alternative spell-three definition when actually supplied.

The native stage snapshot represents original slots64..191, with raw actor
origins at the preceding stage camera, actual identities, generations, hitboxes,
HP and flags. A copied view lets later spirits see earlier flag0x80000 marks within
the same update. Primitive commit validates every newly marked target's original
identity/generation before any pool or mark mutation. The caller must acknowledge
those one-update marks after successful commit before committing the same runtime
again against a later stage. A retained target follows its original raw slot:
a nonzero replacement identity continues steering; identity zero starts departure.
Target acquisition consumes no RNG. The native engine owner preserves the original
global circular cursor when transferring between stage sessions.

Flight/target steering uses retained pre-camera screen positions. Only the later
engine camera translation moves the body, metadata rows and visible trails; room
entry rebases without an extra controller/animation tick. Real native enemy combat
uses the spirit's current world-space attack and original group cooldown across
generations. Hits do not retire a spirit. Ordinary damage, death and EXP retain the
existing stage service. No attack or target readiness is inferred from pending
factories or uncommitted rows.

The pure portal component preserves the original exhausted-allocation branch,
which destroys its root without releasing scene freeze. This runtime instead
returns an actionable fatal error before committing that unreleased freeze. This
explicit failure-path concession prevents an indefinite native hang; it does not
release the request silently or create substitute rows. Optional spirit-row
exhaustion retires that child before spell initialization/RNG. Late malformed-data
or allocation errors preserve the runtime, character RNG/MP and actual stage pool.

Authored public tests exercise lifecycle, real factory delay, distinct Tetra
parameters, flash tail timing, one-shot capture, sixteen-row/eight-packet ownership,
same-update target marks, raw retained-slot replacement, camera ordering, no-tick
rebases, held factories/Fire, row exhaustion, rollback, Wolf replacement, atomic
stage commits and actual enemy damage/death with retained cooldown. Fixtures use
synthetic geometry and deliberately different schedules, not private game tables.

Private comparisons match16 composed contexts/3,520 full original player-entity
updates, including actual factory scheduling, child dispatch/frozen gates, attack
initialization, group recount, animation,128-slot target selection, retained trails
and SDK geometry. Both spells/facings, no-target/real-slot snapshots, moving targets,
identity replacement and supplied camera shifts are covered. The original
constructor/dispatcher/child/animation/target routines execute. Real allocation
leases are modelled at the primitive boundary; GPU flash, framebuffer copy, sound,
RNG, empty-armor query and SDK CLZ use explicit boundary adapters. Player pose,
stage targets and engine translation are supplied fixtures. Debug/ASan snapshots
match; this is not native gameplay, uninterrupted route or console parity.

Selected BlinkWhite profile0x67 also matches336 supplied normal/Bat image contexts
and27,856 full original controller/shared-angle/SDK packet updates in Debug/ASan.
It retains full-parameter deduplication and the actual private image/profile lookup.
No public assets, private traces, relic grants or save-format changes are added.

Provenance: selected `dra/7879C.c:{UpdatePlayerEntities,EntityFactory,
CreateEntFactoryFromEntity,EntityPlayerBlinkWhite,func_8011A328}`,
`84B88.c:{func_80129864,EntitySummonSpirit}`, `5D5BC.c:GetSpellDef`,
`alu_anim.c:UpdateAnim` and `5087C.c` camera translation, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
See [component scope](spirit-spells.md), [casting/freeze](spirit-casting.md),
[stage freeze](stage-spell-freeze.md) and [sound scripts](spell-sound-freeze.md).
