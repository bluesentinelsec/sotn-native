# Chapel stained glass and lighting

The internal Royal Chapel checkpoint now renders its selected original stained
glass, light fills/rays, colored glow, window background and receding ceiling.
Chapel remains disconnected from public castle travel while its confessional,
Maria event and Hippogryph encounter need integration. This is not a complete
stage or authentic-game milestone.

The actor adapters follow US `st/e_stained_glass.h` and its subdivision/palette
helpers at the pinned reconstruction. Private DAI records supply model vertices,
subdivision indices, twelve panel poses, Y rotations, colors, background anchor
and translation/rotation records. No original model, palette or pose arrays enter
public source or packages.

The glass actor owns sixty primitives. Its first update initializes twelve panel
templates; subsequent updates project original poses through the selected SDK
matrix/divider adapter. Near panels split recursively into two vertical portions
with the original rounded model/UV midpoints and horizontal visibility test.
Right-side panels also present a white fill, projected light texture and colored
glow. Preserve the original additive and quarter-additive texture-page modes,
individual priorities, private colors and screen-relative anchors.

The background actor owns nineteen primitives for mirrored windows, their fills,
the floor gradient and paired ceiling panels. The US source explicitly questions
parts of this routine. The native code retains its packed-coordinate midpoint
arithmetic, including low-word carry and negative-X sign extension, using defined
integer operations instead of aliased `RECT`/integer storage. It also retains the
US assignment that copies the initialized fourth color back over the third floor
color. These are explicit console-comparison items; do not silently replace them
with visually preferred behavior during authenticity work.

Subdivision and lighting share a checked primitive cursor. Invalid private poses,
models, indices or excess output fail clearly before an out-of-bounds write or
the source's unchecked exhausted-pointer dereference. Initial pool exhaustion
omits the optional actor presentation as the source does. Private rotations and
models stay bounded to the selected normal-castle paths.

## Palette ownership

The background requests sixteen original RGB555 interpolations on initialization.
The core emits a copied `SotnPaletteBlend` command; the session applies it to the
renderer before drawing. The renderer validates all source colors and destination
bounds, calculates every result, then commits to its owned texture memory.
Transparency and mask bits survive, and borrowed private data remains unchanged.
Ticks and room entry clear the request, so ordinary updates do not repeat it.
This is a selected palette service, not a general PS1 GPU implementation.

The actor phases, generated palettes and primitives remain transient. Version-eight
ordinary saves and player progression are unchanged.

## Validation and remaining work

All 93 test groups passed in Debug, macOS ASan/UBSan and Release; packaging passed.
Synthetic tests vary model geometry, poses, colors and subdivision traversal. They
cover SDK projection, original UV rounding, light/blend modes, background geometry,
packed midpoint behavior, palette command reset, invalid inputs, allocation/output
limits and source/destination ownership. Renderer tests check exact interpolation,
transparent/masked colors, missing final source color and failed-write atomicity.

A private native LLDB sequence runs both placed actors in the original interior
room for 120 ticks and 30 regular rendered frames, with camera walking. A diagnostic
temporarily hides and restores each component: 20,325 pixels differ for glass/light
and 13,430 for the background. These checks establish visible contributions from
their actual private textures, palettes and rendering paths. Full-art captures were
visually inspected. Both Debug and sanitizer native sequences pass.

Shared private regressions also pass four tower crossings and three Wall/Library
connectors, isolated ordinary save/Continue and failed-destination owner retention.
Existing user save slots remain untouched. Explicit room starts, protection and
component masking are controlled fixtures, not ordinary traversal, uninterrupted
castle replay or console parity. Dummy audio tests ownership/decoding rather than
listening or PCM parity. Confessional/scripts/boss, connected Chapel progression
and full audiovisual/GPU/timing parity remain incomplete.
