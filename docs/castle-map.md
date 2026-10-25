# Exploration map checkpoint

Tab/M or Xbox Back/Select opens the map when the player can pause. Select or
Start closes it. The map reads the existing ordinary save's exploration/purchased
bits, rebuilds discovered blueprint tiles, marks the player, and redraws opened
secret passages from permanent flags. Opening/closing never reveals unvisited
cells or changes exploration counts. Renderer calls do not advance game state.

The original F_MAP.BIN blueprint, common palette/cursor texture, and DRA secret
passage records remain private runtime resources. The new optional file has a
recorded US-dump fingerprint in `config/reference-us.json`; it is loaded and
verified only when the map is first opened. Existing checkpoint starts do not
require it. No rendered map pixels are saved or distributed.

Reveal and passage logic follows `dra/5087C.c` (`func_800F1B08`,
`RevealSecretPassageOnMap`), orientation/fade follows `dra/624DC.c`, at
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later. The native scratch
owner replaces VRAM reads/writes and clips the blueprint edge instead of allowing
out-of-bounds reads. Normal/reversed orientation, purchased/revealed overlap,
Caverns blueprint corrections, and secret-passage edges are reconstructed from
state; original artwork and passage tables are never embedded in source.

This remains selected map integration. It uses an eight-tick native fade and the
original pulsing cursor calculation; complete GPU fade/rasterization parity is
unproven. The current checkpoint freezes actor simulation while the map is open;
the original map engine can still update selected stage presentation. Laboratory
supports the normal castle; reversed-map decoding is covered synthetically, not
claimed as playable reversed-castle support. Whole-game map parity remains work.

Public synthetic tests cover reveal/purchase overlap, permanent secret edges,
reversed orientation, invalid progress/resources/capacity/opacity, atomic failure,
and input mappings. A local LLDB run checked opening, closing, fixed simulation
time while open, resuming play, and rendering private map data.
