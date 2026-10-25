# Long Library runtime foundation

Long Library (stage 2) now has a native resource profile, selected shared actors
and its own breakable behavior. Ordinary connected travel stays disabled while
its area-specific enemies, Lesser Demon, librarian/shop and scripts remain pending.
There is no separate public Library gameplay start. Loading a room or rendering
its artwork does not establish a working Library route or console parity.

The profile validates 24 sprite banks, original actor IDs below 0x4f, eleven
persistent prize entries and their castle-flag base, one red-door layout, camera
metadata and save room 14. It resolves shared tables through Library offsets and
rejects missing mappings. Seven complete camera-property records are backed;
the original hitbox/axis declarations have eight entries. The later local camera
unlock signal remains outside this slice. Stage-start graphics bank zero loads
independently from each room's graphics bank. Its sound-bank files are separately
fingerprinted; original Wood Carving Partita intro/loop music uses the existing
private stream. No runtime tables, artwork or sound data enter public files.

Selected Skeleton/body/bone/piece services use Library initialization and tables.
The original Library layouts contain no ordinary Skeleton placements; the later
Lesser Demon summoning path remains unintegrated. Flea Men use their Library
motion/timelines/sensors, actor identity and damage/contact path. Other placed enemies and background/script actors still warn explicitly and remain omitted.

The overlay's retained Flying Zombie code now runs its linked legs/torso, lower-
and upper-body hit branches, flight/hover/dive and independent deaths. Original
first-Library layouts contain **no Flying Zombie placements**; this adapter is
not additional ordinary route coverage. Adjacent parts use native slot/generation
ownership; an attached orphan retires instead of following a reused slot. Spawned
children start with the original whole-pixel positions and cleared fractions.
Its original blood children retain the eight-record/two-sprite allocation budget,
private five-byte UV streams, fixed-point movement, SDK Z rotation, color and
priority, and the packed control-fraction camera adjustment. Drips retain the
original additive gradient line, gravity and old-head tail calculation. Optional
pool exhaustion omits effects as the original does. No blood artwork, palettes,
lookup tables or animation rows are embedded in source.

All ten original breakable types now read their private animation, palette,
texture, hitbox and effect profiles. Hanging lamps retain their adjacent helper;
extinguished helpers retain the original parent-ID rule. Ordinary candle drops
require active Cube of Zoe. Urns, jugs and busts produce their selected persistent
prizes independently of that relic; busts retain their original inert debris.
Optional exhausted actor pools retain the original allocation-failure behavior.
Selected lamp sounds preserve original screen-position volume/pan calculations.
Do not replace these rules with the earlier areas' breakable implementation.

Library room 14 uses ordinary v7 saves and the existing fifteen-slot UI/Continue
path. Its supported inventory, permanent prize flags, relics, time and event
records use the current format; no application or save-version bump is required.
The native slot browser identifies it as LIB. Fresh diagnostic entries retain the
explicit development profile; they are not original new-game initialization.

Public synthetic tests cover table/identity bounds, disabled travel, initialization,
shared animation, all breakable branches, Cube gating, persistent prize/debris
selection, optional pool exhaustion, positional sound, malformed inputs, default
graphics upload and stage-aware save roundtrips. Private LLDB checks use protected
explicit room starts. The corrected Flea fixture starts above the floor and verifies
HP loss, the death flag, explosion conversion and a recorded kill; disappearing
from view alone cannot pass. Debug and macOS ASan/UBSan finish at tick 24 with seven
native renders. A separate positioned-contact check reduces the protected hero's
HP by nine; it does not establish ordinary traversal. A second sequence injects hit parameters into original lamp, bust
and urn placements, draws their effects/pickups, then explicitly starts save room
14, uses its ordinary save UI in a local test slot and completes fresh Continue.
Both builds draw forty frames and retain the collected castle flags. These are
component checks, not ordinary combat-input coverage for every candle, uninterrupted
travel, unmodified-character playtests or audiovisual/console parity evidence.

Metadata, shared behavior and breakables adapt pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later):
`st/lib/{header,e_init,e_breakable,e_lock_camera,e_collect,e_skeleton,e_flea_man}.c`,
shared stage headers and `dra/lba_stage.c`. Native data ownership, profiles and
save/session adapters remain C code in this repository. Private inputs stay ignored.
Selected Flying Zombie/blood algorithms additionally adapt `st/e_flying_zombie.h`,
`st/e_bloody_zombie.h`, `st/prim_helpers.h`, `st/e_misc.h` and `dra/5087C.c` from the
same pin. Public synthetic checks cover paired ownership, both split paths,
HP resets, fixed-point death direction, timer/hover/dive behavior, exact effect
positions, quad UV carry, camera fractions, original line-tail ordering and pool
exhaustion. Private native LLDB fixtures explicitly insert this retained enemy
into room 6 with a protected hero. Ordinary attack/crouch/jump inputs finish the
lower-hit path at tick 1621 with 406 renders and the upper-hit path at tick 853
with 214 renders, both in Debug and macOS ASan/UBSan. They check the death sound
when the root retires; mere loss of an actor cannot pass. These fixtures do not
prove an ordinary Library encounter, complete GTE saturation, uninterrupted
traversal, unmodified-character playability or audiovisual/console parity.
All 61 public groups pass in Debug and macOS ASan/UBSan; release packaging passes.
