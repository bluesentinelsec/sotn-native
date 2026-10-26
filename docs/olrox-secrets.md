# Olrox's Quarters secret wall and ceiling

The internal NO2 checkpoint now runs the placed secret wall in room 2, its
adjacent-room debris in room 3, and the breakable ceiling in room 15. Attacks,
collision-tile replacement, selected debris/sounds, discovery, ordinary saves and
re-entry use the original NO2 rules. Other mechanisms, scenery, Olrox and castle
connections remain incomplete; this does not make the area complete.

## Behavior and persistence

The wall accepts three hits, retains the original infinite-HP sentinel, and waits
sixteen ticks after each of its first two hits. Its cracked frames, thirty-two-tick
reveal delay, split graphics and falling fragments follow the original state
sequence. Flag 32 mask `0x01` records discovery; mask `0x02` records completed aftermath.
Entering the adjoining room can finish that aftermath independently of the old
wall actor. Preserve these two bits and the different allocation-failure paths.

The ceiling starts with 128 HP, the original rotated presentation and closed
tiles. Hits scatter particles. Physical defeat replaces eight private tile entries,
sets flag 33 and launches the selected fragments. Re-entry restores opened tiles
without recreating the barrier. Transient fragments and particle lifetimes are
not ordinary save data.

The shared hit service now preserves `0x7FFF` as infinite HP while retaining hit
notifications. Finite enemies and the ceiling still receive normal damage.

Version-nine saves already retain both discovery bytes. The format stays the
same; validation bounds the wall's two bits and the ceiling's boolean. Existing
map presentation reads the original private passage table and these flags for
explored cells. No second exploration store or public passage data is introduced.

## Data and native boundaries

Tile indices/replacements, launch/rotation profiles, artwork and SDK lookup tables
remain private. These patches use each room's actual indexing, unlike the separate
96-tile indexing in some NO2 spike operations. Validate the entire tile patch
before writing any of its entries.

Fragments retain generation-owned display/metadata pairs and original free-pair
allocation order. Activating a pair leaves its old displayed geometry until the
next simulation projection. Motion fractions, gravity, collision-before-movement,
secondary fragment creation, bounces and allocation-dependent RNG follow the
original sequence. Native structs replace packed primitive-field aliasing.

The ceiling's initial thirty-particle allocation must be consecutive. Its final
zero-width tile is invisible, but its lifetime, packed fractions and RNG still
advance. Native presentation omits that zero-area draw. Subsequent fragments use
ordinary linked allocation, with bounded ownership checks and cleanup.

The selected fragment projection retains identity X/Y rotation, Z rotation and
equal depth/screen distance 1024. Full GPU behavior, audio/reverb and elemental
presentation parity remain incomplete. No private arrays or diagnostic captures
are included in source or release packages.

## Validation

All 108 Debug, macOS ASan/UBSan and Release test groups pass, along with
asset-free release packaging. Native LLDB checks pass in Debug and macOS
ASan/UBSan; automated runtime checks now use silent SDL dummy output unless
specifically testing real audio.

Public synthetic tests cover physical attacks, infinite/finite HP, hit recovery,
reveal timing, complete tile-patch bounds, both discovery bits, opened re-entry,
fragment cleanup, particle exhaustion, the different allocation fallbacks and
malformed patches. Save tests retain the two bytes in version nine and reject
invalid discovery values without replacing decoded state. Map tests use authored
coordinates to prove that the two flags reveal their respective passage edges.

Private component comparisons cover 19,964 root/primitive snapshots across wall,
ceiling and both adjacent-room aftermath cases. The original US instructions run
with isolated initialization, collision, allocation, map-reveal and sound adapters;
selected projection operations use the pinned SDK reference. Compared fields
include state/timers, tile hashes, discovery bytes, motion/fractions, rotation,
fragment dimensions, displayed coordinates/UVs, RNG, sound and effect counts.
Injected hit/death flags establish component behavior, not an original encounter
or a console replay.

A separate native LLDB check starts the ordinary development character near the
placed wall, protects it, and uses normal Gladius attacks. It opens the wall and
crosses room 2 to room 3, retaining aftermath flag 3 at tick 153 with 39 rendered
frames. It then uses the actual save-room UI with an isolated slot, loads fresh
version-nine state and verifies that the barrier stays open on an explicit revisit.

The ceiling check positions the same unmodified character for each normal Gladius
air attack, allowing it to land between attacks. It does not inject actor hits,
HP or death. It opens the ceiling at tick 1267, renders debris, saves/loads through
the isolated save-room UI and verifies opened re-entry. The combined check renders
401 frames. Positioning stands in for the currently missing movement ability;
it does not prove that the character can reach the ceiling through ordinary play.
Explicit save-room/revisit transfers are not walked routes. Existing user slots
remain untouched. Neither check establishes whole-area, ability or console parity.

Provenance: `st/no2/e_secrets.c`, selected `st/prim_helpers.h`, original hit sentinel
handling in `st/st0_hit_detection.h`, and NO2 identities at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
