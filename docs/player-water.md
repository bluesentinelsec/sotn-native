# Selected native player water feedback

The native session now copies the actual preceding stage controller's independent
feet, waist and head depths into Alucard before his next update. It uses the
[Entrance water owners](native-water-actors.md), rather than treating a collision
tile as a volume. Ordinary starts grant no relics and version-nine saves do not
store these transient depths.

Any nonzero foot depth supplies the original no-afterimage and water status bits.
Selected normal, hit-recovery and petrified gravity quarter their acceleration beyond40 pixels. Collision
motion scales both velocities by3/4 before movement, then restores by4/3 with the
original signed wrapping and truncation. Its threshold is41 pixels normally and
13 pixels for Wolf. The original player entity byte0x49 suppresses this scaling
and clears afterward; it is separate from the damage-invincibility timer. Selected
ordinary producers leave that byte zero. Stuck/grab status retains the original
collision boundary exclusion. This does not establish complete state/collision
parity beyond the supported player paths.

Waist depth supplies existing Mist/Bat entry and return gates. Wolf's existing
Holy Symbol ownership rule uses waist depth; its controller receives waist and
foot values separately for swimming and falling. Shared player particles now
receive actual foot depth even when Wolf is absent. The existing Wolf head,
deferred factory and particle owners consume the resulting underwater bubble
requests. No body, relic, MP or visual readiness is manufactured by a depth setter.

Depth setters never advance simulation. Menus and spell freeze retain their
existing boundaries; room changes clear depth context without an extra player
update. Stage currents continue to commit after the player update through the
existing motion owner, preserving previous-stage sampling and actual actor order.

Public authored tests cover shallow/deep movement and gravity, threshold12/13 and
40/41, status/snapshots, collision-byte reset, separate damage immunity, signed
rounding/wrap, invalid-copy rollback, room clearing and actual Wolf controller
depth/swim mapping. Private comparisons execute8,372 original status and pre/post
collision-scale contexts, plus864 original gravity contexts. The scale comparison
supplies the collision boundary; it is not a full player-frame replay. Debug and
ASan/UBSan traces match byte for byte. Native private room fixtures exercise the
actual stage/player/effect owners with explicitly identified protection and grants.
The water-room check rejects Bat, Mist and Wolf without spending MP, then enables
owned Holy Symbol Wolf, verifies separate body depth providers, swims and creates
original bubbles through actual factories and their private seven-row leases.
It uses explicit room/position/relic/contact-protection and head-cadence fixtures,
not an ordinary-acquisition or full-route playthrough.
The actual CMake-built Debug app also verifies preceding-stage depth sampling and
real afterimage resets under muted LLDB, then exits normally through SDL quit.
Selected [normal standing/crouching bubbles](normal-water-bubbles.md) now use their
own original shared DRA counter and real particle factories.

Water damage, water death, further state/equipment effects, other stage-specific
water owners, whole-route and console parity remain incomplete. In particular,
this checkpoint does not substitute ordinary physical recoil or death for the
original water-damage path. Alucard and the castle remain unfinished.

Provenance: selected `dra/692E8.c`, `6FDF8.c`, `wolfForm.c` and original particle
consumers, pinned `sotn-decomp` revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`
(AGPL-3.0-or-later). Tables, assets and comparison traces remain private.
