# Selected playable Library Card

Owned hand item166 now uses its original medicine definition, ordinary hand input,
player18 and private F4/F5 animation. Standing, walking and crouching accept it,
including while cursed. Airborne states and Axe Armor do not. The command birth
retains its original velocity; the next teleport callback clears both velocities.
The item has no weapon overlay, substitute sprite or MP cost.

Consumption occurs once on successful command birth. Unequipped counts exclude
the copy in the hand: spare stock decrements while the hand stays equipped; using
the last copy empties that hand and recalculates actual character stats. Owned
Duplicator84 in either accessory preserves both stock and the equipped copy.
Its selected supported producer is this Library Card use, not other consumables.
The character update commits only after player animation, collision, afterimages
and position checks succeed. Native failed-player handling restores any actual
factory reservations. Whole-stage HP, clock, map and audio atomicity remains open.

Original pose8/timer1 reserves actual factory121 from the shared eight roots.
Root exhaustion holds timer2 before animation returns it to1 for the next retry;
it neither refunds nor repeats consumption. The actual child65 owner then retries
its36-row allocation before96 birth RNG calls. Optional or unrelated actors are
never replaced to make room.

Existing teleport callbacks bypass pending healing, player timer decrements,
logical input sampling, contact reactions and ordinary command/form selection.
Physical input and the native engine continue. Original status18100000 and
protection12 apply; the common no-afterimage helper retains its own timer15 reset.
The controller writes padSim0/demo4 on every callback, including recovery. Pending
healing holds until normal control or an original full initialization clears it.
HUD resources and buffs keep their separate actual gameplay phase/status owners.
Inventory pause and repeated draws hold player/effect simulation and game clocks.

After actual shared row commit, child65's palette requests hide the live player.
Its outgoing mode6 selects original RoomTeleport39 and the integrated Library
stage2/room5. The record's source/reload marker31 is retained as metadata; it is
not checked against the departing stage as if this were a door. Candidate session
construction preserves the old session/save store on resource failure. Successful
handoff retains inventory, ordinary connected state and the one shared point array.
Actual arrival initializes death vectors before reserving incoming121/high1,
selects18/sub1/F5 and forces the original left facing. It adds no child update.

The engine owns the original departing FFFF/incoming2 white global-tile requests.
Prepared engine frames consume their counter once; repeated draws do not. These
tiles have no player readiness or arrival timer role. Incoming child65 alone
emits terminal ACK/sound636 and releases its real rows. The next player callback
recovers, leaving the original four-frame simulated-input hold.

Original UpdatePlayerEntities clears retained DRA attack hitboxes after callbacks
on outgoing18/sub0 and death. Selected shared projectile/weapon/spirit/Wolf owners
and subweapon owners now clear their actual active fields, as well as this frame's
combat masks. A later callback must write a new hitbox to restore it. Full familiar,
weapon and shared-runtime coverage remains unfinished.

All six original teleport controller substates and the TOP/RTOP integer alignment
helper are reconstructed. Other-castle triggers and native TOP/RTOP stages are
unavailable; their component contexts are not playable route evidence. Global GPU
capacity/ordering, full audio timing and broader sharing remain separate open gates.

Authored checks cover both hands, spare stock, both accessories, curse ordering,
birth movement, timer/input/healing holds, real root exhaustion, failure atomicity,
incoming ACK/recovery and private-record validation. Private comparisons match
12,000 complete original controller contexts and1,600 complete ReduceWeapon/
equipment-count contexts, with identical Debug/ASan traces. Factory allocation and
stat recalculation are explicit service boundaries in those comparisons. Native
and actual CMake application checks separately exercise owned selection, ordinary
hand input, real effects, connected Library handoff, incoming recovery and draws.

Actual native checks also restore real reservations after a late animation failure,
retry eight occupied incoming factories without replacing them, and clear a real
thrown axe's retained attack field while its motion continues. Supplied frozen
Soul Steal checks retain disabled fields through repeated updates and retire the
real child on release. Actual CMake apps hold the full runtime/all16 words/clock
through inventory open/hold/close, verify two full white61440-pixel engine frames
and quit through SDL. Ordinary enemy2B death still completes capture/64 uploads
to real game over after these common-owner changes.

Owned-item/protection and supplied failure/contention contexts are explicit
fixtures. They establish no ordinary Card/Duplicator acquisition, uninterrupted
castle route or console parity. Default grants and ordinary savev9 stay unchanged.
The [Alucard milestone](alucard-status.md) remains open at32/34 states and all six
workstreams.

Provenance: `dra/6E42C.c:func_8010E42C/func_8010EDB8`,
`dra/71830.c:PlayerStepTeleport/func_80115C50`, `dra/5D5BC.c:ReduceWeapon`,
`dra/692E8.c:EntityAlucard/AluInit`, `dra/7879C.c:UpdatePlayerEntities`,
`dra/5087C.c:RunMainEngine` and `dra/4DA70.c`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
