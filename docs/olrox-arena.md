# Olrox arena foundation

The internal stage-30 BO0 adapter loads the user's verified arena overlay,
textures, sprite banks and sound bank. It initializes default graphics bank zero
and retains the original silent stage-music selection. The actor registry uses
BO0 identities and table addresses; it cannot fall through to Laboratory combat.
Common pickups, red doors, camera locks and selected room decorations use their
shared native services with BO0's private tables.

The paired boss doors now initialize, create the reserved left mate when Alucard
crosses the original entry threshold, move inward, push the player, and replace
their collision tiles. The original completion signal reopens the tiles and
retracts both doors. Motion retains original fractional words, strict endpoint
comparisons, tick-based sound requests and positional attenuation. The child
factory clears fractional coordinates and rejects overwriting unrelated slots.
Tile patches validate every destination and definition before writing.

The completion signal belongs to the arena overlay lifetime. It survives a room
reset but is neither synthesized from missing actors nor exposed as a completed
boss/save event. Boss behavior must drive it when integrated. The comparison also
identified and corrected reversed wall-contact bits in the shared solid-object
collision service: pushing Alucard right reports contact on his left, and pushing
him left reports contact on his right, matching the original player flags.

## Validation and limits

Public synthetic tests cover both door sides, reserved child ownership, strict
entry/endpoints, fractional motion, collision displacement and contact flags,
open/closed tile patches, complete-patch validation, unavailable child slots,
invalid state/data, completion lifetime, private-table metadata, decorations and
explicit unsupported boss identities. No original fixture arrays are published.

A private original-US instruction comparison checks 7,624 snapshots of doors,
movement, child creation, tile mutations, positional sound requests and normal
player collision. Initialization, player-sensor and audio-backend adapters are
isolated; the door, movement, factory, attenuation and collision instructions
execute from the user's dump. Removed actors are compared as removed, rather
than requiring native deferred cleanup to mimic immediate PS1 entity clearing.

Muted native Debug and macOS ASan/UBSan LLDB checks start explicitly inside BO0,
walk through the original entry threshold, render the placed door/mate, verify
the seal, inject an explicit component-only completion signal and verify both
doors retract. Equivalent checks launch the actual CMake-built app through LLDB
and use normal SDL quit. These fixtures do not grant stats, relics, equipment or
ongoing player positions. User save slots remain untouched. Private inputs,
captures, scripts and instruction traces remain ignored.

The selected [humanoid body](olrox-first-form.md) now adds its introduction,
movement, physical damage and attack commands. BO0 and NO2 remain disconnected.
Attack children, transformation, second form, final defeat/reward and connected
progression still need native integration.
Several BO0 shared enemies, scenery and effects are also unsupported. The pinned
reference's central Olrox routines are assembly stubs and require reconstruction;
this foundation does not substitute an approximate fight. The checks above do
not establish an authentic boss encounter, ordinary castle entry, uninterrupted
route, GPU/PCM/frame parity, or complete castle/Alucard support.

Provenance: `boss/bo0/e_boss_doors.c`, the BO0 stage header/symbol metadata and
selected shared room/camera/collision algorithms at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later). Initialization,
animation, tile, sprite and sound data are read only from private runtime files.
