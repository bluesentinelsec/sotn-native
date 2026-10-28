# Shared Wolf body and effect ownership

The selected shared ability owner now updates the fourteen Wolf body entities
alongside actual Bat factories and effects. Wolf entry explicitly replaces the
original slots17..30 before creating generation-owned parts. All allocators see
the same entity mask and copied snapshot of the actual stage primitive pool.

Factories update first. The body then updates at its actual slots17..30, before
later children. BlinkWhite uses its factory-selected slot, not a fixed weapon slot;
the selected fixture allocates slot63. Its Wolf geometry therefore reads the
current body's capture bounds. Running all flashes before the body would use
stale bounds. Body feedback reaches the player once after a successful update;
front-part MP charges commit once through the same character transaction as RNG.

The whole primitive commit validates Bat, Echo, Wolf root and common capture
environment reservations together before mutating the stage pool. It can recycle
retired rows between these owners in either direction while rejecting duplicate
plans, stage collisions and invalid lineage. Binding lasts until full stage reset.
An unbound legacy Bat owner preserves separately owned Wolf rows. Body retirement
releases root rows but retains the common environment until stage initialization.
Camera/room rebases move actual parts and packets without advancing simulation,
animation, capture bounds or MP.

Public authored regressions poison previous capture bounds to verify that the
later flash consumes current geometry. They also cover row recycling, complete
pool rollback, late-child failure after body advancement, and reversible rebases.
A private muted LLDB fixture uses this repository's player, shared owner and real
stage pool through entry, steering, MP, return and retirement. It grants only Soul
of Wolf to the ordinary Laboratory character and writes no saves. It rejects
duplicate body feedback. This is a session-level integration check, not a native
controls playtest or console replay.

Native scene/render scheduling, live controls/relic switching, water and cross-form
integration, real spell/group/combat ownership and child factory/effects remain
incomplete. Spell and bubble reports remain intents. No Wolf controls are enabled
by this service; ordinary starts grant no relics and saves remain version nine.
Do not call Wolf, Alucard or the game complete.

Provenance: selected original `dra/wolfForm.c`, `7879C.c` and entity update ordering,
pinned sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Original inputs, traces and captures remain private.
