# Relic and permanent pickup progression

Stage relics now read their name, icon, palette, and familiar category from the
private DRA definitions. Contact collects ordinary relics with their original
found/active bits and familiar cards with their found bit. The icon floats using
the original timing. The complete orb glow and acquisition banner remain incomplete.
Owning an unsupported movement relic does not silently enable an invented ability;
transformation, double-jump, and relic-menu integration remain separate.

Fresh Entrance checkpoints no longer grant Cube of Zoe: it is collected in its
original Entrance room. Candles break without dropping their prize until Cube is
active. Explicit Laboratory development starts retain their documented Cube grant.
A magically sealed Laboratory door now checks an active Jewel of Open; its complete
message and magical effects remain incomplete.

Laboratory and Entrance permanent pickups use their original private prize tables
and castle pickup bits. Reentering an area, crossing stages, or continuing an
ordinary save preserves collection. Enemy/candle transient drops remain transient.
[Version 6 saves](native-saves.md) retain relic ownership and migrate earlier
checkpoints without removing their previously granted Cube behavior.

The algorithms follow `dra/5D5BC.c`, shared `e_collect.h` and `e_sealed_door.h`
at the pinned sotn-decomp reference, under AGPL-3.0-or-later. No private text,
artwork, definitions, prize tables, or game bytes are checked in.

Public regression fixtures cover definition bounds, original ownership bits,
floating-orb contact, familiar collection, sealed-door gating, v6 saves, frozen
v5 migration and malformed records. Private LLDB checks use original Cube artwork
and collection, followed by a fresh isolated-slot load. These are focused checks,
not an uninterrupted route or original-console parity replay.
