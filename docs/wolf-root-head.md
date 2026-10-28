# Wolf root and head components

The core reconstructs the original US Wolf root (slot 17) and head (slot 20).
These are internal components, alongside the [active controller](wolf-controller.md)
and [linked followers](wolf-articulation.md). Actual transformation, shared
factory/combat/primitive ownership and rendering remain unwired; live Wolf
controls and relic switches remain disabled.

The eventual native owner must preserve original entity order: root 17, front
18, rear 19, head 20, spines 21..29 and tail 30. Pass original screen-relative
fixed-point positions to the root; its 0..256 edge rules are not world-map bounds.
The room collision adapter must translate those queries to the actual room map.
Native camera/room rebases must preserve fractions and retained visual geometry.

The root consumes actual six-primitive allocation results. A failed first
allocation retains the pending child and applies only the original preceding
turn-distance write. An initialized root that loses its real reservation fails.
It computes private pose-dependent length, SDK angles and quantized distance,
ordered floor probes, slope/frame choices and ledge slides. Preserve whole-word
position wrap, original fractions, player/body drawing words, asymmetric screen
edges and signed image-cycle wrap. Negative image selections select no retained
image. The [six-row image component](wolf-root-effect.md) now reconstructs its selected
packets; they still require their actual capture/primitive/render owner.

Root collision searches are bounded to fail clearly on a nonterminating supplied
map; invalid tile-top distances, out-of-range SDK geometry, malformed private
frames and lost ownership fail atomically. This does not replace a bad floor
with a guessed surface or establish complete physical collision integration.

The head attaches to the preceding front body, retains original turning exits,
reads private bob/bite offsets and uses SDK rounding. It reports the original bark,
spell-four charge or spell-fourteen bite initialization, and underwater bubble
factory origins in that order. These requests create no hitbox or successful
factory. The owner must connect attack power, groups, collision, damage and actual
children. Breathing retains the original timer and strict water threshold.
Head animation advances separately after behavior, following the original private
six-record stream and end hold. Turning clears the original animation flag and
retains its selected pose instead.

Public authored tests cover allocation retry, flight origins/fractions, screen
edges, signed cycle wrap, ledges/slopes, original query order, bounded failures,
head initialization/turning, animation hold/restart, bark/attack/bubble requests
and malformed full-width offsets. Private muted LLDB comparisons cover 16,384
supplied root/collision/capacity contexts, including 4,040 supplied first-allocation
failures. Original root/helper/private-frame/SDK instructions execute with explicit
collision, allocation and six-row presentation adapters; only SDK GTE leading-zero
instructions are emulated before continuing original square-root instructions.
A separate comparison covers 49,152 head updates/animations, 28,627 attack intents
and 464 bubble requests, executing original private tables, SDK trigonometry and
`UpdateAnim` with explicit sound/attack/factory adapters. Debug and ASan/UBSan
traces are identical. Supplied collision/body contexts are not native form ownership,
ordinary acquisition, uninterrupted movement or console parity.

Provenance: selected `dra/wolfForm.c:func_8012F894`, `func_8012F83C`,
`func_801309B4` and `dra/alu_anim.c:UpdateAnim`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
All original pose/frame/animation/SDK tables and traces remain private inputs.
