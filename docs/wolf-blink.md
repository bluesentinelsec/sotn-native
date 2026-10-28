# Wolf transformation BlinkWhite component

The core now reconstructs selected original US Wolf entry/return effects and the
Wolf finishing flash, alongside the existing Bat, Fire and Wing effects. These
are effect components. Live Wolf controls remain disabled pending actual body,
transformation, shared factory/combat/primitive, capture and rendering ownership.

Supply the actual current bank-fifteen form flag, copied body capture bounds and
preceding owner signal. Entry parameter 0x23 uses its saved ordinary/Bat image
before signal two, then switches to the articulated Wolf capture. Return 0x24
waits for the actual restoration signal before contracting. Preserve strict
amplitude limits, wrapping wave/angles and original signals; allocation failure
holds phase zero without geometry or readiness.

The Wolf capture path uses raw body bounds, forces unmirrored geometry, samples
the original direct-color page and retains the US signed V offset and byte wrap.
Signed stripe arithmetic preserves inverted supplied bounds without normalizing
them. Saved ordinary/Bat images retain their original rotation and pivot behavior.
Bat entry from Wolf also uses its original wider displacement limits. The native
owner must supply actual saved artwork and the shared capture; components create
neither a body nor captured pixels.

Entry finishes through parameter 0x25 while retaining its existing palette.
Return changes to 0x1B and applies the original white palette. The Wolf finishing
flash holds on the original status bit. Preserve separate Bat/Wolf locks,
class-precheck order and which opposing lock each interrupted effect clears.
Missing images and incompatible forms clear both only at their original checks.
Shared locks must belong to the actual session, not independent form copies.

Public authored tests cover entry/return/sensor waits, finishing palette/hold,
allocation-independent state gates, raw/inverted capture bounds, nonmirrored UVs,
ignored Wolf sprite rotation, cross-form displacement and mutual lock ordering.
Private muted LLDB comparisons cover 960 supplied ordinary/Bat/Wolf contexts and
52,560 complete controller/shared/quad frames, including allocation retries,
entry/return, cross-form entry, common Fire/Wing packets and interrupted locks.
Original image lookup, profiles, SDK geometry, handshake and afterimage-helper
instructions execute with explicit allocation/destruction/GTE-leading-zero
adapters. Debug and ASan/UBSan traces are identical. Separate original normal/Bat
allocation comparisons retain 288 contexts and 20,448 complete frames.

All 146 public tests pass in Debug, Release and macOS ASan/UBSan; packaging passes.
Muted native Bat/Mist, Wing, Fire, Echo, capacity, travel/save and actual-app
regressions retain their documented fixtures and scope. These checks establish
no live Wolf, ordinary acquisition, full shared contention, route or console parity.
Original artwork and all runtime traces stay private.

Provenance: selected `dra/7879C.c:EntityPlayerBlinkWhite` paths and the existing
SDK rotation service, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
