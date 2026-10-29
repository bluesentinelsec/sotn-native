# Selected playable Dark Metamorphosis

The native game now connects the original Dark Metamorphosis command to its owned
player state, MP, learning, artwork, shared factories, palette and blood healing.
Relative to the facing captured at the first input, enter Back, Up-back, Up,
Up-forward, Forward, then press either hand attack. Keyboard attacks are Z/D;
Xbox attacks are X/B. The original standing/walking and hand-recovery gates apply.
It requires 10 MP, including an exact-cost cast, and needs no relic or prior learned
spell. A successful cast learns the spell through ordinary character progress.

The cast uses original state 32 and animation BA, stops motion, removes the actual
shield cursor and preserves the original cancelable hand-recovery word. Normal
standing dispatch now uses the actual attack substep rather than treating any
retained recovery word as an active hand animation. The original animation ending
returns to normal standing. Casting refreshes the selected original invincibility
request; its supported status also freezes ordinary MP recovery while casting.
The absorption duration is 1024 + 4×INT successful simulation updates. A new cast
replaces it. Actual Bat/Mist/Wolf entry cancels it; ordinary damage does not.
Room changes rebase a captured cast source without advancing its timer or effects.

The real shared owner consumes blueprint 40/profile 17, with all six original child
parameters, generations and stage-row lifetimes. The original voice and MP sound
requests use existing native audio. The selected global casting flash retains the
private byte profile and odd-frame pulses: nine opaque complete frames for the
ordinary profile. Repeated drawing never advances it. This ephemeral GPU tile is
separate from retained stage primitive leases; full global GPU packet contention
remains unmodeled.

The normal player glow now reserves original palette job 23 in the same actual
32-job array used by selected Mist jobs. The owner reads private descriptors and
absorption color pairs, changes only its owned player color 10, and retains job
phase/delay across cast, expiry and cape refresh. Original RGB555 stepping retains
the mask bit and waits for an unchanged update before switching targets. The
previous frame's palette work runs before the next engine update, including pause;
spell duration and gameplay remain paused. Draw calls never tick palettes. Source
DRA and artwork remain immutable. Other shared palette jobs and complete GPU/frame
timing parity remain incomplete.

Actual [cutting-hit blood](enemy-blood.md) can now contact an absorbing player and
queue 8 HP, or 16 with owned/equipped Bloodstone. The following player update applies
one pending heal, updates ordinary character/HUD HP and produces both real healing
flashes and pickup sound. Follow blood into its original contact window; its
launch direction follows the struck body's original offset rule, not the attacker.
Floating HP numbers, other normal spells, complete weapons/familiars/water/effects,
all-ability contention and full Alucard/route/console parity remain incomplete.

Ordinary starts grant no item or relic. Saves remain version nine: learned-spell
progress persists; temporary duration, blood and visual owners do not. Full stage
initialization and Continue restore normal form and palette.

Public authored checks cover both facing commands, exact/insufficient MP, unavailable
owners, retained hand recovery, shield removal, animation return, status/timer expiry,
no-tick rebases, malformed-data rollback, palette stepping and real job capacity,
unrelated Mist jobs, drawing from the owned color and shared outline/flash lifetimes.
Private instruction comparisons match 864 command/constructor contexts (384 casts),
10,240 glow updates, 10,240 original flash-tail updates and 5,472 status snapshots.
The constructor executes original CastSpell, INT duration, animation, shield helper,
palette and flash-profile code; factory/sound/destruction/learning calls use explicit
boundary adapters. Palette GPU upload and status helper calls have their own adapters.
These scoped comparisons do not establish an entire original frame. Debug and
ASan/UBSan traces match.

Private muted native checks use explicit Gladius, X 124 entry, HP 54 and contact
protection, ordinary INT/MP/enemies and an optional owned Bloodstone fixture. They
cast through normal input, retain all six outline owners, render nine complete
flash frames, hit placed enemies, absorb actual blood for 8/16, run both healing
flashes and expire the original 1048-update duration. They grant no relic and write
no save slots. These protected absorption sequences are not an unmodified encounter
or route replay.
The actual CMake application separately completes the protected 8-HP sequence and
normal SDL quit in Debug and ASan/UBSan. Native checks also verify paused simulation/
buff duration with continuing glow, repeated-draw stability, and an explicit
in-memory transfer to save room 23, v9 encode/decode and fresh normal-form restore
with learned progress retained. No ordinary slots are overwritten.

Provenance: selected `dra/{704D0,6E42C,71830,692E8,7879C,4A538,alu_anim}.c`,
pinned sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
All original animation/blueprint/color/flash definitions and traces remain private.
