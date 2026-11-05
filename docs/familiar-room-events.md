# Original familiar room-event queue

The US shared familiar event queue now has a bounded component owner. It reads
the actual private templates, tracks their49 timer/link records and emits births
for the original first-empty slots5..7. This does not supply native familiar
slots, activation, hints, effects, audio or progression. All six
[Alucard gates](alucard-status.md) remain open; ordinary grants and savev9 are
unchanged. Native integration must commit the real slots and dependent owners.

The templates remain immutable private data. Their mutable timer/link fields
have one corresponding runtime record each, including the original dummy queue
head. This storage is neither an entity pool nor replacement primitive capacity.
Initializing a newly loaded overlay starts these fields at zero. Same-servant
and switch reset clears only cached room/servant values; timer/link contents
survive. Drawing, menus and other held clocks must not call the updater.

Rebuilding uses the original cached servant and room coordinates. Stage alone
does not trigger a rebuild. US normal/inverted room signs, familiar filters,
exact camera positions, castle flags and relic found bits retain their original
order. A conditioned entry reads the actual existing world flag/relic owner.
Combined conditions short-circuit in the original order. Bounds failures never
substitute a cleared flag or missing relic.

Room entry preserves immediate zero-delay births, delay-minus-one initialization,
retained queue order and the original removal/repeat rules. Later callbacks hold
timers while the camera or condition does not match. Expiration creates a birth
only on the following callback after reaching zero. Repeating entries reload
their original delay; one-shots leave the queue even when optional slots are full.
Omitting an exhausted birth neither replaces an unrelated actor nor invents a
retry/cooldown. Plans capture actual first-empty reservations so the next birth
in the same callback sees the changed occupancy.

The native owner still must preserve the actual parent generation, full origin
fractions, facing, priority and callback ordering when committing each planned
entity. A plan is not a completed allocation. Invalid private records, conditions,
links or cycles preserve both runtime state and output, including a late failure
after an earlier planned birth. Transient queues and clocks are not saved.

Authored tests cover immediate/delayed/repeating events, allocation exhaustion,
cache reset, stage/cache distinctions, camera/condition holds, combined flags,
relic found-versus-active bits, malformed final sentinels, cycles and late rollback.
Private comparisons execute36,704 complete original `ProcessEvent` calls across
376 contexts and all47 original game-event rows. They compare every49-record
timer/link, cached key, actual original slot birth and captured parent/fractions/
flags. The original local creation/destruction code executes without service
adapters. One context per row explicitly changes its private one-shot flag to
exercise repetition; that is a supplied metadata fixture, not ordinary gameplay.
Debug and ASan traces match. All218 authored tests pass in Debug, Release and
ASan/UBSan; Python tests, repository policy, formatting, static SDL linkage and
Release packaging pass. Both actual CMake-built apps also pass the owned-medicine
regression through shared effects, resources, menu/draw holds and retirement.

These are queue/component checks, not native familiar lifecycle, ordinary room
traversal, acquisition or console evidence. See the
[Fairy controllers](fairy-controller.md) for their producers and remaining native
CD/XA, primitive, slot and rescue dependencies.

Provenance: `servant/process_event.h`, `shared_globals.h` and
`create_event_entity.h` (US), pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Private event payloads, source images and raw traces are not published.
