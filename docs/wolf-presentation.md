# Wolf multipart presentation and shared capture

The core packet generator reconstructs selected original bank-fifteen multipart
sprites, including cropping, palette offsets, facing, source rotation pivots and
SDK rounding. It emits normal/capture duplicates in original emission order and
retains normal-before-capture GPU exhaustion. Raw textured packets ignore vertex
colors; their unused colors are initialized rather than copied from scratch bytes.
The owner supplies actual current capacity and validates entity generations.

The renderer applies capture packets to the owned 63×63 VRAM rectangle at
(512,448). Preserve ascending original priorities, reverse emission order at ties,
the status-controlled clear bit and the untouched outer row/column. Failed draws
roll back only the interval they opened. A failed begin must not cancel another
owner's active target. Capture uses the current articulated image, not a frozen
image for each retained-position effect. The original US late writes occur after
the capture packet copy and do not change the already-emitted capture attributes.

These services do not allocate player/body entities or environment primitives,
advance simulation, signal readiness or enable Wolf controls. Actual transformation,
MP, body generations, collision/combat and shared render ordering remain to be
connected. Ordinary starts grant no Wolf relics and save format remains version nine.

Public authored tests cover geometry, pivots, facing, capacity, invalid private
references, atomic failures, ordering, clearing/retention and capture-edge texels.
Private muted LLDB traces compare 5,400 supplied contexts and 8,716 emitted packets
against original RenderEntities instructions, private multipart definitions, SDK
rotation and actual ordering-table/capacity logic. A GTE leading-zero adapter and
supplied GPU/CLUT state bound that comparison. Debug and ASan/UBSan traces match.
A separate supplied-body fixture uses actual private artwork, the room collision
adapter, original root/follower/head services, six root plus two environment stage
rows and the current capture. It writes no ordinary saves. These component checks
establish no live Wolf, ordinary acquisition, whole-form, route or console parity.

Provenance: selected `dra/4B758.c:RenderEntities` bank-fifteen paths and
`dra/692E8.c:func_801093C4` environment, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later. Artwork,
original tables, runtime traces and captures remain private inputs.
