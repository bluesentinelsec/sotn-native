# Selected Bible

The Bible (subweapon 5) now runs through the shared player runtime, after
[Vibhuti](player-vibhuti.md), [Rebound Stone](player-rebound-stone.md) and
[Agunea](player-agunea.md). Hold Up and tap either attack button. Defaults, grants and saves are
unchanged.

## Use

The shared `func_8010EB5C` path ([Vibhuti](player-vibhuti.md#use)) with `g_SubwpnDefs[5]`:
5 hearts, chain 1, blueprint 77 (`E_SUBWPN_BIBLE`, entity 43, slots 32..47, origin 0: the
player's position at use) and standing animation 0x5D.

## Book

`EntitySubwpnBible`:

- **Birth.** One GT4 (page 0x1E, palette 0x17F, 16x24, opaque, hidden), flags
  `KEEP_ALIVE_OFFCAMERA | HAS_PRIMS | FLAG_UNK_20000`, a tilt swing of -32 (32 facing left),
  then `func_8011A290` (attack roll, least-used group 7..10) and a 6x6 hitbox.
- **Reveal.** The next update shows the book and sets timer 10 = 4. The radius then grows by
  one per update to 0x30.
- **Orbit.** Each update rotates the `rcos`/`rsin` orbit by 0x80 (the other way facing left),
  tilts it by the swing (which reverses at +-0x200) and places the book around the player;
  it draws two above the player's priority while behind and two below while in front.
- **Launch.** After 0x12C counts it loses `KEEP_ALIVE_OFFCAMERA`, launches at 12 pixels an update
  forward and upward, falls by 2 per update (starting in the same update) and plays
  BIBLE_SCRAPE. It is destroyed once it leaves the screen window.
- While shown it requests `FACTORY(79)` every update and plays the Bible swish whenever
  `g_GameTimer` is a multiple of ten (after the scrape in the launch update).

The US build reads its third rotation term (`var_s4`) uninitialised. No function on the gameplay
call chain, from `UpdatePlayerEntities` up to the executable's start routine, writes that
callee-saved register, so it holds its value from boot. Zero, as the PSP/`FIX_UB` builds
initialise it, gives the visible orbit and is what the native port uses; the private comparison
confirms a nonzero value would move the book.

## Trail

`EntitySubwpnBibleTrail` (entity 44, blueprint 79, slots 32..63 from the top, subweapon id zero):
one half-transparent GT4 (page 0x1C, palette 0x19D, 16x16) placed once at the book's position.
Its grey starts at 0x60 and drops by eight each update; it is destroyed on its eighth callback.

The book and its trails have neither `FLAG_POS_CAMERA_LOCKED` nor `DRAW_UNK02`, so they keep
their screen position while the camera moves, and `FLAG_UNK_20000` keeps them across room
entry (without the room translation). Pending trail factories are retired on room entry.
Factory entities themselves have no `FLAG_POS_CAMERA_LOCKED` in the original either; native
factory sources do not yet follow the camera between request and spawn (one update).

## Validation

Public tests cover the birth, reveal, an exact orbit (rotation, tilt, priority), the swing
reversal, launch and fall, the trail's fade and life, and the runtime's factories, trails,
camera anchoring, room retention without translation, and the off-screen cull. A private
emulator comparison runs the original `EntitySubwpnBible` and `EntitySubwpnBibleTrail` (with the
real `func_8011A290`, `rcos` and `rsin`) on the actual DRA/SLUS against the native callbacks over
300 contexts (81,474 callbacks) with nonzero scroll; a deliberate mutation is caught. A private
LLDB probe uses the Bible in the Laboratory by pad input both ways; the book orbits, trails,
damages a held zombie, launches and is culled. These use explicit
subweapon/hearts/protection/position/enemy-HP fixtures, not acquisition, route or console
evidence.

Adapted source: `src/dra/8A0A4.c`, `7879C.c` and `4DA70.c` at the pinned reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
