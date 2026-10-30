# Ordinary water-controller transaction

The core reconstructs the placed US water controller's cache, current displacement
and ordered splash/surfacing plans. It still enables no native water feedback,
swimming or damage behavior. Its [volume query](water-volume-query.md) supplies the
original private geometry and three distinct depths.

Birth caches the query; the native owner must initialize the placed actor. Depth
feedback starts on the next update. The controller preserves whole-word vertical-velocity tests,
entry/exit thresholds, prior versus current contact rows, slope/edge packing and
the original entering-variant overwrite. Stage-created children start with zero
fractional coordinates, matching `CreateEntityFromEntity` rather than the separate
player-factory copy rule. Currents use original signed-word scaling, wall masks
and fixed-point displacement; cached sampling positions precede current movement.

Child plans use an actual occupancy snapshot for original slots224..255. They are
not completed copies, owned children or successful allocations. The native owner
must validate unchanged availability and commit every real reservation before the
copied controller, cooldown or depth/current feedback commits. No second actor or
primitive pool is introduced. Failed optional allocation stops a side-splash chain;
only a reserved surfacing child starts its cooldown, which decrements in that same
update. Draws, menus and frozen stage updates retain their separate engine owners.

Public tests use authored tables for birth, entry/exit, packed parameters, partial
exhaustion, surface cooldown, fractional-origin clearing, currents, wall blocking
and rollback. A private comparison runs 1,296 supplied form/slope/current/velocity/
capacity contexts over51,840 original updates, checking controller cache, depth
globals, currents, cooldown, allocation attempts and744 actual created-child
snapshots. Original query, initialization and creation instructions execute;
auxiliary allocation/destruction use explicit adapters. The contexts supply player
motion and occupancy, rather than claiming native traversal or effects rendering.
Debug and ASan/UBSan traces match byte for byte.

The [water-effect components](water-effects.md) now reconstruct the child packets
and particle behavior; they still require real native ownership.
Actual placed-controller, splash/drop/primitive ownership, native player feedback,
water damage and uninterrupted-route/console parity remain separate work. Ordinary
starts and version-nine saves stay unchanged; Alucard and the castle are incomplete.

Provenance: `st/water_effects.h:EntityAlucardWaterEffect` and stage
`create_entity.h:CreateEntityFromEntity`, pinned `sotn-decomp`
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later). Original tables,
images and comparison traces stay private.
