# Long Library runtime foundation

Long Library (stage 2) now has a native resource profile, selected shared actors
and its own breakable behavior. Ordinary connected travel stays disabled while
shop interaction and remaining scripts are integrated. The selected
[Librarian introduction](library-librarian.md) now runs its placed trigger, chair,
first speech and player control through the shop-ready script gate. Lesser Demon's selected
[placed arena flow](library-demon-entry.md) now runs entry, combat, seals and reward.
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
private stream. The [Librarian SEQ service](library-sequence-audio.md) now has a
native decoder/voice path; its manager now requests playback. Shop interaction
still needs integration. No runtime
tables, artwork or sound data enter public files.

Selected Skeleton/body/bone/piece services use Library initialization and tables.
The original Library layouts contain no ordinary Skeleton placements; the
Lesser Demon summoning path now creates selected minions. Flea Men use their Library
motion/timelines/sensors, actor identity and damage/contact path. Flea Armor, Dhurons and plant enemies now run their original placed behavior,
attacks and deaths.
Book encounters now retain their selected original behavior.
Other placed enemies and background/script actors still warn explicitly and remain omitted.

Flea Armor retains its walking/leaping attacks, adjacent attack hitbox, dust and
six-piece break-up; the root becomes an exposed Flea Man with the original brief
hitbox delay. Library Flea Men now use their own floor/side sensor tables as well
as their motion and ceiling data. Positional jump/death sounds and the original
transient minion-removal signal are stage-local. Dhurons retain movement,
normal/rapid sword attacks, charging waits, weapon damage/element conversion,
charged palette cadence and seven timed death pieces. The original forty-record
lightning chain reaches its sword, then runs its flash/fade. SDK direction and
trigonometric data remain private. Native child generations retire stale owners.
A re-entering lamp or paired enemy replaces only its own matching surviving child;
unrelated occupied slots and original placements remain protected. This preserves
original fixed-child factory behavior without a re-entry termination.

Thornweed and Corpseweed now preserve wake/idle profiles, the delayed adjacent
head, staged leaf/stalk growth, bobbing, head scale, attack waits and projectile
splat. The grown root changes its original enemy-definition selection while
retaining HP and copied contact power. Native damage attributes and EXP follow
that selection. Root/head deaths remain linked: the falling head contacts the
floor, the leaves/stalk contract and fade, and the root's original timed fade
retires the owned child. Both directions retain their geometry and signed screen-
coordinate averaging. Texture/hitbox/timeline tables remain private; no artwork
or effect frames are compiled into the app.

Ectoplasm now runs its original elliptical motion, reaction to a square-button tap,
hit squash/rebound, falling death, flames and recipe-driven afterimages. Button
taps derive once per simulation tick from the held buttons; holding a button
cannot retrigger a tap. Original SDK trigonometry and distinct libc random draws
remain explicit. Summoned variants retain their fade/blink and stage-local cleanup
signal. Emitters require a current owner generation; independent afterimages
finish their own animation when an owner retires. Their hidden anchor still
reserves its original primitive budget. [Selected curse contacts](character-status.md) now apply the original duration,
failed equipped attacks and recovery, including room/stage timer preservation.
Full curse visuals and complete contact/effect parity remain pending. Library
travel stays disabled while the other encounters and scripts are integrated.

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
The placed enemy batch also adapts `st/e_flea_armor.h`, `st/lib/e_dhuron.c`,
`st/e_flea_man.h` and the selected shared dust helpers from that same pin.
Public synthetic cases cover transformation, hitbox/damage changes, minion cleanup,
fragment lifetimes, charging allocation/fade, independent Library floor/side
profiles and safe fixed-child re-entry with primitive cleanup. Protected native
LLDB room-3 checks use original placements and ordinary inputs: Flea Armor then
its exposed Flea die at tick 286 (72 renders); a Dhuron dies at tick 112 (29 renders).
The separately selected Dhuron charge state finishes its beam/flash and doubles
weapon attack from 15 to 30 at tick 115 (39 renders). Debug and macOS ASan/UBSan
agree and exit successfully. The charge fixture selects a state explicitly;
it is not evidence of ordinary AI entry or a whole-route/console replay.
The plant batch adapts `st/e_thornweed_corpseweed.h`, with original LIB constants
from `st/lib/e_thornweed_corpseweed.c` and private enemy-definition selection
following `dra/5D5BC.c:DealDamage`. Public tests cover wake/spawn timing, both
facing growth paths, attacks, linked head-first/root-first deaths, transformed
attributes, projectile collision/splat, stale owners, pool exhaustion and malformed
pose tables. Native LLDB checks use original room-9 placements and a protected
hero: the Thornweed dies at tick 24 (7 renders); the Corpseweed grows and fires,
then dies through a crouching root attack at tick 670 (168 renders, 204 growth
ticks and 51 projectile actor ticks). Debug and macOS ASan/UBSan agree and exit
successfully. These explicit room fixtures do not establish an uninterrupted
Library route or console/audiovisual parity.
Ectoplasm additionally adapts `st/lib/e_ectoplasm.c`, the selected summoned fade
from `st/lib/e_lesser_demon.c:func_us_801BB8DC`, and the existing direct wide-animation
service. Synthetic tests cover tap/hold behavior, motion excitation, rebound,
death timing, owner-independent tail lifetime, recipe emitters, summon cleanup and
primitive exhaustion. Protected native LLDB combat against its original room-9
placement finishes the observed HP/death path at tick 259 with 65 renders in Debug
and macOS ASan/UBSan. Both exit successfully. That combat fixture causes no hero contact and does not itself validate curse,
ordinary traversal or console/audiovisual parity. A separate positioned-contact,
pause and explicit-arrival fixture is documented in [character status](character-status.md).
All 68 public groups pass in Debug and macOS ASan/UBSan; release packaging passes.

Spellbooks and Magic Tomes now wake, approach from depth, pursue/charge, turn and
open their two halves before commanding their adjacent beam. Their twelve private
textured faces use the original SDK XYZ matrix, signed projection, winding and
AVSZ4 depth ordering. Their center projection supplies the original half-offset
body hitbox and attack origin. Death preserves spin/opening, page fragments,
flames, sounds and the falling body's camera retirement. The beam grows to its
original damage interval, shrinks normally or faster with a dead owner, and
retires stale generations. Explicit zero scale hides its final visible frame;
it cannot become a unit-size flash. Original debug-only book controls are omitted.

Public synthetic book tests cover wake boundaries, both attack cycles, hit recoil,
original signed angle-delta narrowing/remainder, primitive and actor-pool exhaustion,
owner death/replacement, malformed geometry/recipes and checked fixed-point motion.
A private comparison uses the pinned PSY-Z SDK algorithms with the user's original
matrix-pair table: 40,000 randomized face and center projections agree, including
winding, coordinates, saturation and ordering. The backend's compiled pair table
differs from the original game's table; substituting it is not original-data parity.
The selected transform remains bounded to these models, not general GTE overflow.
Protected native room-9 fixtures use original placements and ordinary movement/attack
inputs after observing each active beam. Spellbook death/retirement completes at
tick 1111 (278 renders), Magic Tome at tick 614 (154 renders).
Debug and macOS ASan/UBSan agree and exit successfully. An explicitly selected
attack-state fixture renders all 110 ticks through the visible zero-scale beam frame
and idle return. These fixtures are not uninterrupted traversal, unmodified-character playtests or console/
audiovisual parity. Librarian/shop/scripts and Library travel remain pending.

Selected [Lesser Demon attack services](library-demon-attacks.md) now retain spit/
column and branching bolt effects, with a native colored-quad path for the later
summoning effect. Their explicit fixtures alone do not establish a working boss.
[Selected poison mechanics](player-poison.md) now run; Library travel remains pending.
[Selected body AI](library-lesser-demon.md) now runs the boss's attacks, summoning and
physical defeat; the placed entry/seals/reward sequence now has a separate protected
[ordinary-save check](library-demon-entry.md). The selected
[summoned minions](library-minions.md) now run Mudman and the Skeleton/Ectoplasm
appearance and cleanup paths; their protected inserted fixtures are component evidence.

The [shop transaction foundation](library-shop.md) now reads private stock and
retains equipment/gem/relic/map/scroll ownership changes in v8 saves. Selected
buy/sell menus, speech, exit and returning greeting now run. Full shop panels,
optional services, missing equipment effects and spell casting remain unintegrated;
this service alone does not enable ordinary Library travel.

[Selected Library furnishings](library-furnishings.md) now include the placed
candle table, its owned shadow and permanent Stone Mask drop, ordinary head-equipment
metadata, and original foreground/background decoration records. Sitting and Mist interactions remain pending. Native
explicit-room checks do not prove ordinary Library travel or console parity.

The [selected Library backgrounds](library-backgrounds.md) now project repeated
bookcases/furniture, column shadows and distant illuminated shelves from private
records. The original backdrop and Library foreground occlusion are retained in
the selected native path. Private SDK comparisons and protected explicit-room
checks do not establish whole-route or console rasterization parity.

The [secret bookcase](library-secret-bookcase.md) now runs original push delay,
rotating collision, lighting/subdivision and saved discovery. Its ordinary-character
native check crosses the passage and retains discovery through isolated ordinary
save/Continue; this does not establish whole-route parity or enable Library travel.
