# Selected Library furnishings

The original placed candle table now animates with its adjacent shadow, accepts
ordinary attacks, breaks, and produces persistent item 1. That private item entry
resolves to Stone Mask. Its original inventory/equipment metadata now works through
the existing head-equipment and derived-stat services. Decorative table variants
retain their original static frames. Breaking the table does not award enemy EXP
or remove its broken base, and Cube of Zoe does not gate this permanent pickup.

The shadow uses its original quarter blend and opacity. When struck, the table
replaces only its owned adjacent shadow with the pickup. Re-entry can replace the
same table's surviving shadow/pickup; it never overwrites a placed or unrelated
actor. Explosion exhaustion omits the optional effect while preserving the drop.
The original global effect pool 224–255 maps to native stage slots 160–191,
because the native stage owner excludes the first 64 global actors.

Original foreground and background decorations now decode their private ObjInit
records, animation streams, priorities, texture offsets, palettes and blending.
The selected foreground variant retains its original half-turn. US Library's
ObjInit2 path reads the word at the facing/texture fields into the texture offset;
it does not apply the separate facing correction used by other builds. Invalid
indices, draw flags and bounded pointer references fail explicitly.

No original animation, initialization or item tables enter public source. Ordinary
v8 saves already retain the equipment and original permanent-item castle bit;
this batch does not change their format or the application version.

## Evidence and remaining scope

Synthetic tests cover table/shadow/decorative states, hit-flag semantics, adjacent
owner safety, re-entry, optional effect exhaustion, ordinary Stone Mask equipment
stats, original decoration field decoding/rotation, and invalid records.

A private native LLDB check starts at the actual room-1 table with the ordinary
Gladius development character. An ordinary strike breaks it, the player collects
the original permanent pickup, the native equipment menu equips it, and a v8
codec roundtrip retains the equipment and collected bit. This uses an explicit
room start and a synthetic save-room identity for the codec check; it is not an
ordinary save-room replay or uninterrupted route/console parity proof.

The existing placed Librarian shopping/return/save/fresh-Continue sequence also
passes with these decorations. Parallax backgrounds, the rotating secret bookcase,
ordinary sitting behavior, Mist-form/door interaction, remaining shop services,
complete effects and ordinary Library travel remain separate work. Do not call
the Library complete.

Algorithms adapt pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`:
`st/lib/e_candle_table.c`, `background_block_init.c`, `st/e_room_bg.h` and
`st/e_room_fg.h` (AGPL-3.0-or-later). Runtime data stays private.
