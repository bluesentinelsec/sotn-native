# Doppleganger packed sprite sheets

The actor renderer now resolves BO4 body/bat sprite-sheet definitions as well as
its existing multipart sprites. It reads the original private sheet pointer tables,
packed pixels and pivot metadata, then samples them through the stage's loaded
palettes. Mirroring, transparent/opaque black, selected blend arithmetic, scale,
rotation and opacity use bounded native rendering. No sprite bytes or pointer tables
are embedded or published.

The renderer checks the overlay/header, table extent, frame/sheet indices,
dimensions, pixel extent and all sixteen palette entries before drawing. BO4's body
and bat pointer tables contain 211 and six sheets respectively. Its frame banks
also retain 46 body-frame definitions whose sheet references have no backing entry
in this overlay. Those are rejected; the private sweep skips them explicitly.
They are not counted as supported or as successful validation. Referenced frames
must be checked again when the boss animation/state machine is integrated.

The original high frame tag selects the same pose. This immutable sheet path does
not emulate later modifications to cached PS1 VRAM, including death dissolution.
CPU transform/raster registration, full GPU ordering and other stage sheet formats
still require parity work. This graphics adapter does not activate the boss or open
ordinary travel into the arena.

Public synthetic tests check pivots/mirroring, both sheet banks, the frame tag,
nearest sampling under scale, semitransparency, clipping, malformed indices/pixels
and rejection in an unrelated stage. Private native LLDB checks render 217 backed
body and 16 bat frames with original palettes under Debug and macOS ASan/UBSan;
46 unbacked frame definitions are skipped. These are graphics fixtures, not a
boss fight or console comparison.

The sheet branch adapts `src/dra/4B758.c`, using BO4 header/spritesheet metadata at
sotn-decomp revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`
(AGPL-3.0-or-later). All original tables, artwork and private fixture images remain
local runtime inputs/artifacts.
