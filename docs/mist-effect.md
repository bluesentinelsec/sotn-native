# Mist visual lifecycle

The internal Mist visual cursor reconstructs the original sixteen-point cloud,
eighty textured quads, four color oscillators, sprite fade and contraction back
to a captured player outline. This is an owned simulation component; live player
dispatch, real sprite capture/upload, palette jobs, camera translation and shared
effect allocation still need integration. Mist remains unavailable in gameplay.

The owner supplies eight actual BlinkWhite snapshot wedges at initialization and
again when requesting restoration. The original private DRA weights, flow flags
and facing permutations stay in runtime files. Trigonometry, direction and square
root use the existing private SDK services. No artwork or original lookup array
is embedded. Four original random results initialize the color phases only on
the first capture; reported consumption must advance the owner's RNG once.

The cursor retains original signed-word/fixed-point wrapping, negative-coordinate
division, ring-point fractions and angular phases. Initial ring high-word writes
preserve fractions from the previous effect lifetime. A new entity anchor retains
its own inherited fractions. All eighty outputs use the original modulated,
semi-transparent, dithered draw flags (`0x537`, plus the snapshot hide bit); the
sixteen snapshot triangles use priority +2 and the sixty-four cloud quads +4.
The descriptor supplies texture page, palette, UVs and per-vertex grey values.
It does not implement those renderer flags or upload texture memory itself.

The expanding cloud cancels its palette job at tick 79. Restoration takes a fresh
real capture, converges for 25 ticks, and requests the original palette cancellation
and job `0x11D` at tick 12. Only its resulting phase 5 permits the player controller
to finish returning to normal form. A requested cancellation is not readiness.

Phase 5 increments a signed sixteen-bit timer initialized to 16; it does not
decrement it. The original destroys the effect only when that timer wraps negative.
The native cursor preserves this behavior and retains the last geometry during
that wait. The owner must distinguish restoration readiness from effect retirement
and apply original entity replacement/lifetime rules when integration is added.

Checked table ranges, permutations, geometry arithmetic and SDK lookups fail with
an SDL error before changing the cursor or output requests. Missing captures fail
explicitly. Shared primitive-pool exhaustion, rotated/alternate player capture,
palette/color residency, framebuffer output and whole-form gameplay/parity remain
separate work; a bounded private geometry comparison does not prove them.

## Validation

Public fixtures contain authored linear/constant SDK tables and authored wedges.
They cover UV subdivision, priorities, retained fractions, cloud fade/phase timing,
fresh-capture requirements, contraction, actual phase-5 readiness, signed-timer
wrap, facing/alternate pages and unchanged outputs on malformed input. These
fixtures test lifecycle/ownership and are not original SDK reference data.

A private muted LLDB CMake-library helper and original-instruction comparison
match 1,840 complete snapshots: all sixteen ring points, eighty quad coordinates,
UVs, colors, pages, palettes, priorities, visibility and lifecycle state. Cases
include negative coordinates, retained fractions, both facings/pages, different
outlines, movement, ordinary expansion and restoration through timer wrap.
All 127 test groups pass in Debug, Release and macOS ASan/UBSan; release packaging
passes. Debug and sanitizer traces are byte-identical, and both muted LLDB inferiors
exit successfully. Original geometry, SDK trig, angle and square-root instructions execute. Explicit
adapters supply captured wedges, successful primitive allocation, random results,
palette requests and effect destruction; the GTE leading-sign-count register is
emulated. This is component agreement, not real capture, shared-pool, renderer,
playable-form, uninterrupted-route or console parity. Inputs and traces stay private.

Provenance: `dra/7E4BC.c:EntityMist/func_80121F58`, sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
