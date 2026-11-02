# Selected familiar retirement and row validation

The selected Fairy lifecycle now has an atomic four-slot retirement service.
Ordinary card selection, resource initialization, room/stage/session lifecycle,
persistence and whole-pass rollback remain unfinished. This service does not
activate or deselect a card, reset audio or release FFF; those actual owners must
complete their own transactions. Existing grants, pools and v9 saves are unchanged.

`SotnFairyRuntime_Clear` validates the actual4..7 identities/generations and all
three possible5..7 DD/DF leases before clearing any entity or row. Complete
namespace validation checks every actual row, including hidden sentinels, overlaps
and tagged rows absent from the supplied leases. An empty lease cannot silently
abandon an owned row. Stage/common/shared player rows retain their real owners.

Successful retirement clears only the four entity records. It preserves the overlay
binding, generation serial, history, room-event links/timers and sound cursor for a
later initialization of the same loaded overlay. Full overlay replacement remains
a separate operation. Repeated retirement is a no-op after validation. The native
update skips an empty retained namespace before querying active-card providers;
subsequent ordinary gameplay continues without inventing a familiar binding.

Room translation now uses the same complete namespace validation before any copy
or retirement commits. Previously, its precheck validated only row0 of each lease.
A stale sentinel in a later sibling could make that sibling's commit fail after an
earlier sibling's129 rows had already retired. The regression reproduces that
failure against the previous implementation and verifies that the current path
preserves the entire pool and runtime on failure. Both ordinary translation and
room retirement reject stale ownership before changing anything.

Public checks cover all three real129/10/33-row leases, late stale sentinels,
omitted lease words, unclaimed rows including an otherwise empty namespace,
retained overlay globals, common rows and idempotence. Private Debug/sanitizer
checks reproduce the prior partial-retirement bug and validate actual native
medicine/captured-death contexts:344 retired Fairy rows, six ordered draws and
30239 removed Fairy pixels in each build. Actual shared medicine/death presentation,
resident textures, stage actors, character resources/RNG and clocks survive the
retirement/draw boundary. The next ordinary Step advances normally.

These authored slot/effect/source/protection/owned-item/lethal fixtures establish
selected retirement and rollback. They are separate from ordinary card deselection,
resource initialization, saved progression, acquisition, uninterrupted routes and
console evidence. Original `ServantWorkClear` scans4..63 for D0..DF identities;
this service owns the selected native Fairy4..7 namespace, not unimplemented other
familiar entities or complete shared contention. All six Alucard gates remain open.

Provenance: selected `dra/menu.c:ServantWorkClear`, `dra/7879C.c`, and private
Fairy namespace/effect ownership from `servant/tt_002/faerie.c`, sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Private inputs and probe traces remain outside published source.
