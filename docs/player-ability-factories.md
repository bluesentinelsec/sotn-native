# Original player-ability factories

The core now reconstructs original DRA factory construction, scheduling and shared
slot selection. This service consumes the caller's actual occupied-slot mask and
returns ordered slot writes. It does not reserve a second pool or enable live
Bat commands, spells or familiars. The native session must still reserve actual
factory slots 8..15, own child generations, initialize behaviors and retain original
update order and shared primitive/attack-group contention. [Selected Wing Smash](player-wing-smash.md)
now supplies actual Bat factory/child ownership; all-ability sharing remains incomplete.

Original blueprints and entity ranges are read from bounded private DRA views.
No runtime tables ship in public source. A factory retains its original source
position, facing, priority, inherited update flag, packed parameter/family fields
and signed argument split. Origin rules determine whether later ticks follow
Alucard and whether walking, dive-kicking or damage ending cancels the factory.
The first initialization tick retains the source's distinct cancellation ordering.

Initial delay and cycle delay preserve original tick boundaries. Allocation can
run forward, backward or round-robin across the original three ranges. Critical
factories retry when slots are exhausted; noncritical ones retire after any
successful partial batch. Parameter increments follow either the lifetime spawn
index or the current cycle index. Only the original forced weapon-slot kind
replaces an occupied slot and requests weapon-state clearing. Empty-child
blueprints are valid: an original zero identity clears the selected slot and
leaves it available, including multiple writes to the same slot in one cycle.
Family modifiers still apply before deciding whether an identity is empty.

Validation precedes committed cursor/output changes. Malformed descriptors,
truncated tables, unsupported range kinds and invalid native state fail with an
SDL error. Public fixtures cover delays, forward/reverse/round-robin allocation,
partial capacity, critical retry, optional retirement, forced/empty writes,
parameter continuation, signed argument split, packed families, first/later origin
behavior and atomic errors. Live factory reservation and child initialization
remain separate responsibilities; returned writes do not assert successful combat
or visual completion.

All 135 public test groups pass in Debug and macOS ASan/UBSan; Release
packaging passes. Private muted LLDB comparisons exercise all 123 original blueprints in 3,936
supplied source/occupied-mask contexts, producing 12,346 scheduled records and
6,324 ordered slot writes. Original construction, free-slot scans and scheduling
instructions execute; entity destruction uses an explicit adapter. Player states
and pool occupancy are controlled fixtures, not a live shared-child replay or
console parity. Debug/sanitizer traces are byte-identical and exit normally through
muted LLDB. Original images, tables and traces remain private.

Provenance: `dra/7879C.c:CreateEntFactoryFromEntity/EntityFactory` and
`get_free_entity.h`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
