# Early Short sword progression

The Short sword collected in the upper Entrance can now be equipped through the
ordinary equipment menu after Death removes Alucard's opening gear. Its original
family-37 overlay supplies standing, crouching and airborne attack frames,
hitboxes, sound requests and artwork for either hand. Inventory ownership and
ordinary v9 saves retain the equipped item without a new save format.

The [shared ordinary hand owner](shared-ordinary-hands.md) now supplies its actual
factory48/slot16 callback, character property RNG, hand group and native rendering.
Its per-hand previous-pose BSS follows actual program lifetime and stays out of
ordinary saves.

The weapon's five palettes are checked and loaded as five palettes; adjacent
animation bytes are not treated as extra colors. The [Jewel sword, its special and
the Stone sword](player-jewel-stone-swords.md) share this overlay. This integration
grants no weapon or stats during ordinary travel or Continue.

Public synthetic tests cover metadata rejection, active attack variants in both
hands, ownership/equip/restore, and both-hand rendering with a bounded five-palette
view. Private native LLDB checks exercise rendering and physical enemy damage
with an explicitly granted test item. Those fixtures do not establish uninterrupted
route or console audiovisual/timing parity.

Adapted from `src/weapon/w_037.c` and the existing player attack services at
sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under AGPL-3.0-or-later.
All original definitions, attack/sprite tables, palettes and sounds remain private.
