# Fairy slots, shared effect leases and scene rendering

The Fairy runtime now owns the original four-slot namespace4..7, using the
existing [controllers](fairy-controller.md), [room events](familiar-room-events.md),
[effect components](fairy-effects.md) and [native CD service](familiar-cd-audio.md).
The native scene can draw its actual root/wings and generation-checked shared
effects. Ordinary card selection, gameplay update binding, item consumption,
saved progression and FFF/player17 rescue remain unfinished. All six
[Alucard gates](alucard-status.md) remain open; grants and savev9 stay unchanged.

Root4, wings5 and event slots5..7 have actual identities and unique native
generations. Wings require their current root generation. Initialization keeps
the original0/2, switch1 and same3 distinctions; same3 resets event cache keys
without replacing live slots. Event6/7 and retained globals survive root/wings
initialization. Fresh overlay ownership starts the original sound-step word at99,
matching its actual data initializer. Initializing it to zero would stall the
first introduction before any speech event could be created.

The caller updates4..7 in order with the original pass's stable scene gate and
current per-slot snapshots. Ordinary0 runs callbacks, frozen3 holds slots without
flag10000, and FFF runs the familiar callbacks. This supplies the familiar
dispatcher portion of FFF; full player17 and the native engine path remain open.
Controllers retain their own original animation calls, without another generic
animation update. Unbound generic engine-animation flags fail explicitly.

Local event creation precedes room plans in the selected callbacks. Both use the
same first empty5..7 slots, capturing full source fractions, facing, priority and
parent identity. Exhaustion replaces no unrelated actor and does not suppress an
item-spending request. Room-special and sound-state callbacks use the actual
runtime history and live CD snapshots. The caller must commit resource/factory,
scene, pause, sound and other native requests before advancing the next slot;
CD10/11 must change the actual audio owner's acknowledgement flag immediately.
The whole-pass checkpoint remains a native integration responsibility.

DD/DF effects reserve the real shared stage rows. A lease records its actual event
slot, generation and row indices, including DF's sentinel. Replacement validates
every previous row before allowing reuse, rejects stale/incomplete/colliding
plans before mutation, and prevents a live generation from silently resizing its
lease. Retirement releases only that event's rows. Common player and Bat/Wolf
owners share the pool and retain these leases across their own commits.

Familiar medicine requests reserve actual55 roots8..15 with original arg2 zero
and no hand bit. They capture the familiar's room-space source, rather than the
player's source. Ordinary hand medicine keeps its separate item argument and
Hammer/Life Apple restrictions. This bridge does not consume inventory or make
Fairy item use available by itself.

The native scene draws bank20 root/wings, DF tiles and DD light through its
priority ordering, clipping and current target. Original screen coordinates get
target/shake offsets once. Draws validate the complete effect lease, including
the hidden sentinel, and do not advance controllers, RNG, audio or rows. DD's
untextured Gouraud path honors original DTD before RGB555 conversion and blend;
textured/flat DTD, arbitrary offscreen origins and full GPU timing remain outside
this selected service. Unexpected DD offscreen targets fail explicitly.

Public authored checks cover four-slot birth/order, parent identity, same-servant
retention, fresh sound-step99, frozen/FFF dispatch, actual effect birth/retirement,
late bounds rollback, shared-pool collisions/stale generations/sentinel ownership,
factory exhaustion and raster-coordinate DTD. Earlier controller/effect instruction
comparisons retain their own scope; they are not a new whole-dispatcher or console
comparison. Native checks separately exercise the real stage pool, scene drawing,
private bank20/bank2 resources and actual CD backend. Supplied initialization,
level/load-count, frame gates and origins are fixtures, not ordinary selection,
progression, acquisition, uninterrupted routes or completed rescue.

Both native builds pass the1,600-frame introduction/following context:3,340
actual slot callbacks, three real CD commands,40 ordered scene draws and10,561
sprite pixels, with25 frozen pass holds and repeated draw holds. Both also pass
all eleven private DF profiles and DD through complete lifetimes and actual
scene drawing:762 nonretired callbacks,109 draws,1,047,360 changed pixels and328
DTD differences. These counts describe controlled service contexts. All220
authored tests pass in Debug, Release and ASan/UBSan;34 Python tests, repository
policy, formatting, static SDL linkage and Release ZIP checks pass. Existing
owned-medicine checks pass in both actual CMake-built apps.

Remaining work includes native gameplay/lifecycle and resource bindings, camera/
player-lock/room translations and cleanup, inventory and saved familiar stats,
whole-pass rollback, actual8..63 rescue cleanup/resource restoration, full FFF
player updates and player17 recovery/Blink4A. No substitute pool is introduced.

The subsequent [inventory/translation/recovery-flash services](familiar-inventory-translation.md)
supply actual bag-byte consumption, no-tick camera/player/room operations and
finite Blink4A. Their native gameplay/lifecycle callers and player17 recovery
remain unfinished; the earlier availability boundary above is historical.

Provenance: selected `servant/tt_002/faerie.c`, common servant helpers,
`dra/7879C.c:UpdatePlayerEntities/func_8011A870` and original primitive flags at
sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Runtime game data and all source comparison traces remain private.
