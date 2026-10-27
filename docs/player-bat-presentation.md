# Bat presentation foundation

The native libraries now decode original Bat artwork and run selected original
Bat entry/return BlinkWhite effects. These components are not yet connected to
playable Bat movement, collision, damage, commands or input. Soul of Bat remains
unavailable in the relic controls; ordinary starts grant no new abilities.

## Artwork and ownership

Bat uses private DRA bank thirteen and ARC's separate Bat pointer table. It does
not reuse normal Alucard's sheet table. The decoder validates frame pointers,
packed dimensions and pixel residency and returns borrowed pixels. Zero is empty;
the original null table entry fails explicitly. The runtime owner must apply the
original live settling offset to the Bat pivot; the decoder preserves immutable
startup metadata rather than modifying private DRA bytes.

The owned texture service now separates packed-player upload from Mist's snapshot
geometry. Normal/Bat uploads retain the original right/bottom clear margins and
validate the display slot before mutation. They require no Mist color/geometry
profile. The existing normal Mist capture keeps its own profile and fan checks.

## Actual transformation effect

The selected BlinkWhite cursor owns eight original textured strips. Parameter
`0x20` runs entry distortion; `0x21` runs return distortion. They emit original
owner signals two and three from their actual amplitude/geometry phases. The
return effect waits for the player's real sensor-restoration handshake. A supplied
timer never substitutes for readiness. The owner must still reserve real children
and primitives, suppress duplicates, upload the displayed sprite and run children
in original order.

The automatic finishing flash retains the same child and primitives, switches to
parameter `0x1B` and uses private color profiles. Its original `0x7008` condition
raises brightness quickly and holds while the player retains the corresponding
restoration status. Shared phase angles and the Bat-effect lock are explicit owned
state, including removal of an obsolete finishing flash when a new transform locks.

Geometry preserves facing, signed-word wrap, negative origins, strip division,
private RGB phase/divisor ordering, the initial priority and copied draw flags.
Rotation uses the original SDK square root, angle and quarter-wave tables and
the renderer's `FLT(128)` rounding term. This means 128 in twelve-bit SDK units;
it is not the game's sixteen-bit movement scale. Rasterization, PS1 dithering and
complete shared effect/factory contention remain separate work.

Start and Select now use the original US PS1 words `0x0800` and `0x0100` internally.
The early menu constants occupied shoulder-button bits needed by transformations.
Enter/Xbox Start and Tab/M/Xbox Back retain their documented user controls; save
format nine contains no serialized pad state and remains unchanged.

## Validation

All 131 test groups pass in Debug, Release and macOS ASan/UBSan;
release packaging passes. Authored fixtures cover Bat table separation, exact borrowed pixel bounds,
malformed/null entries, original pad words, effect lifecycle/signals, delayed
sensor handshake, finishing-status holds, locks and atomic failures. Existing
Mist upload/capture checks still pass.

A private comparison covers 96 supplied player/image contexts and 6,672 complete
effect frames: every nonempty Bat frame, normal artwork, both facings, four
rotations, three screen origins and two settling offsets. It executes original
image lookup, SDK transform geometry, private profiles, shared phase/color updates,
handshakes and the afterimage helper. Primitive allocation/destruction and SDK
leading-zero instructions use explicit adapters. These are controller/geometry
comparisons with supplied player handshakes, not a playable Bat or console replay.
Debug and sanitizer traces agree. Muted LLDB checks of both actual CMake app
builds open/close pause and map using the original internal pad words, then quit
normally. They use the ordinary character without grants or user save writes.
Original artwork, tables, screenshots and comparison traces stay private.

Provenance: `dra/7879C.c:EntityPlayerBlinkWhite`, `4B758.c:func_800EB758`
and sprite-sheet rendering, `692E8.c` Bat pivot handling, `d_2FE48.c` metadata and
`pc/pl_arc.c` table loading, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
