# Selected deferred healing numbers

Actual deferred Mail, blood and Soul Steal orb healing now creates the original
green number that expands above Alucard, travels toward the HP meter, pulses and
shears away. Both original heal kinds use type1. The number shows the requested
amount before the HP cap, including full HP and stone; stone still suppresses
the separate flashes. Dead and spell-frozen players retain pending healing
without applying it or creating another number. Sound and Blink ownership retains
the existing distinction between blood/orbs and silent Mail.

`CreateHPNumMove` uses direct entity19 allocation in the first available original
slot56..63, rather than a blueprint factory. This owner shares the actual child
mask and generation cursor with other abilities. Its motionless birth reserves
twenty rows from the real stage primitive pool: four private textured digit GT4s
and sixteen green one-pixel trail tiles. Child or row exhaustion omits only this
optional visual, without changing committed healing, sounds or flashes. It never
replaces an unrelated child or uses a separate pool.

The component retains the twelve original phases, least-significant-digit-first
layout, captured pre-controller fractional origin, signed-word geometry, private
SDK angle/root/trigonometry, descending trail births and twelve-update tile hides.
The selected type1 lifetime is a birth plus108 updates. Its initialized flag10000
advances during spirit-frozen effect frames; the separate release-only frame,
menus, map and repeated draws hold it. Camera/room translations update retained
coordinates and packets without another animation step. Offscreen coordinates
do not cull this keep-alive child. Retirement releases its actual twenty rows;
full stage/Continue replacement clears transient children. No resources, default
grants, ordinary-save fields or version-nine format change.

The player retains the original requested amount only after a successful update,
with the existing HP/pending-heal transaction. Shared-runtime failures preserve
its copied children, RNG and reservations; primitive commits reject stale or
duplicate ownership before mutation. Supplied SDK inputs above the supported
nonnegative signed-word square-root domain fail explicitly. Other original number
types and producers, complete damage/equipment/familiars/shared contention,
Alucard, castle and console parity remain unfinished.

Public authored tests cover digit boundaries, phases/lifetime, retained fractions,
signed wrapping, tile birth/hide, optional exhaustion, first-free slots, real row
commit/retirement, frozen updates, no-tick rebases, invalid ownership, late private
data failure and the player's capped/full-HP/stone amount and rollback boundaries.
Private comparison matches392 supplied contexts and42,728 updates against the
original controller and all four GT4/sixteen tile packets, with SDK CLZ,
allocation and destruction adapters. Private data, tables and traces remain
outside public source. These comparisons are not a full frame or console replay.

Muted native checks use actual equipped Mail walking128, deferred HP40→41, direct
slot56/entity19, twenty real rows and a private digit contributing five frame
pixels. Menus/draws hold the child; an actual Summon sequence advances it on33
frozen effect updates and holds its release-only frame. An actual Soul Steal
command/global attack/orb sequence produces five eight-HP numbers, including
healing at the HP cap, then retires their children. Explicit Mail, health,
protection and Soul HP50/MP50/position fixtures are separate evidence from ordinary
acquisition, uninterrupted traversal or console parity. No user saves are written.

Provenance: selected `dra/7879C.c:CreateHPNumMove/EntityNumberMovesToHpMeter`,
`dra/692E8.c` deferred healing and `dra/7879C.c:UpdatePlayerEntities`, pinned
`sotn-decomp` `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`
(AGPL-3.0-or-later). SDK root scaling uses the existing private-table MPL-2.0
math boundary; definitions, artwork and SDK tables remain private.
