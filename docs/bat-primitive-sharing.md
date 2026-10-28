# Selected Bat primitive sharing

Bat entry/return and finishing flashes, Wing/Fire flashes, Wing trails and Echo
now reserve rows from the native stage's existing 1,024-row primitive pool.
BlinkWhite owns eight rows, each trail owns one, and Echo owns forty-nine. These
Bat effects use no second pool. Retained geometry stays with each actual child;
the pool tracks its generation, original kind and actual reserved row indices.

US BlinkWhite retries phase-zero allocation when eight rows are unavailable.
It produces no packets, geometry progress or readiness signal while waiting.
The actual child remains pending; releasing capacity resumes its original entry
or return controller. A finishing flash retains its existing lease through its
parameter change. Trails retire immediately when their single-row allocation
fails. Echo retains its original sound-then-retire failure behavior. Factories
and their actual child-slot pressure remain separate from primitive pressure.

Each update plans against a copied occupancy bitmap and commits the complete
selected Bat reservation after successful simulation. Invalid indices, duplicate
rows, malformed child ownership or stage/foreign-ability collisions leave the
actual pool unchanged. Retirements release planned rows in original child-slot
order. Matching numeric stage and player generations cannot read or free each
other's rows. Room entry preserves selected leases; full stage initialization
and Continue retire transient owners. Room/camera rebases advance no extra tick.

Public authored tests cover exhaustion, delayed readiness, trail retirement,
fragmentation, atomic releases/commits, collision failures and unrelated-owner
preservation. Private muted LLDB comparisons exercise 288 supplied normal/Bat
image contexts and 20,448 complete controller/shared/packet records, including
zero-, one- and nineteen-frame allocation failures. Original image lookup,
profiles, transform/SDK geometry, colors, handshake and afterimage helper execute;
allocation, destruction and SDK CLZ use explicit adapters. This is a controlled
component comparison, not a console replay. Debug and ASan/UBSan traces are identical.

The native capacity fixture fills actual stage rows, holds the real Bat child
pending, releases those rows, and verifies the same generation completes entry,
return and cleanup. It uses an explicit capacity fixture and Soul of Bat grant,
with ordinary stats, start and enemies. Existing muted native Wing, Fire, Echo,
room and Gas Cloud cross-conversion checks retain their separate fixture scopes.
Original inputs and traces remain private; ordinary user saves are untouched.
These native sequences and separate CMake-built app runs pass in Debug and
ASan/UBSan through muted LLDB, with normal cleanup and inferior exit zero.

Mist and other legacy player effects still need actual primitive sharing. Some
legacy stage effects pack original drawable/data pairs into native records;
this work does not establish full original allocation contention. Global factory/
attack groups, afterimages, full effects/audio, Wolf and whole-route/console parity
remain incomplete. Do not call Bat, Alucard or the game complete.

Provenance: selected `dra/7879C.c`, `4DA70.c` and `8BEF8.c`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
