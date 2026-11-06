# Selected CalcDefense terms and additive body records

The native character now matches DRA's whole `CalcDefense`, and seven body items whose records
are purely additive are available. Defaults, grants and saves are unchanged.

## CalcDefense

Defense sums both hands' and all five body slots' defense, then adds `SquareRoot0(CON) - 2` and
the DEF buff, clamped to 0..999. The element masks OR the body items' weakness, resistance,
immunity and absorption with the buff and relic terms. Three terms were missing before:

- **Shield rod (item4)** beside a shield (category 9) in the other hand adds two defense.
- **A held Medusa shield (item13)** grants stone immunity (`ELEMENT_STONE` 0x200).
- **A held Fire shield (item15)** grants fire immunity (`ELEMENT_FIRE` 0x8000).

While those shields are held, the existing US `HandleDamage` prepass applies. Stone immunity
strips the stone element from a contact: it still damages but no longer petrifies. Fire
immunity gives the immunity response, with no damage. These are equipment terms, separate from
the shields' guards and spells.

## Additive body records

Lightning mail (11), Ice mail (12), Dark armor (16), Holy mail (18), Ring of Varda (74), Ring of
Feanor (81) and the Covenant stone (86) are now supported. Their records carry only:

- defense;
- element weakness, resistance and immunity;
- stat bonuses;
- accessory attack.

The existing owner already reads all of these. The original references none of them in code (DRA
or stages), so equipping them changes stats and elements only.

The other unsupported body items have special behavior (cape palettes, room-count defense, item
drops, enemy HP display and the like) and stay unsupported until their owners exist.

## Evidence

- Public tests cover Shield rod's +2 beside a shield, each shield immunity (and its absence with
  the other shield), Ice mail's resistance and the Ring of Varda's bonuses.
- A private MIPS oracle runs DRA's real `make_all` on 4,000 sampled contexts: shields, Shield
  rod pairs, the new armors and accessories, the Alucart set, hours, Vlad masks and base stats.
  It matches every stat bonus and total, both hands' attack, defense and all four element masks.
  Debug and ASan traces are byte-identical. The oracle found the missing Fire shield immunity
  and catches a deliberate mutation of the Shield rod term.
- A mutation sweep catches all nine mutants of the new terms.
- A private native LLDB probe in a real Laboratory session equips the shields through the actual
  equipment menu. The player's attributes gain stone immunity from the Medusa shield and fire
  immunity from the Fire shield. Shield rod and Ice mail raise defense, and Ice mail's resistance
  reaches the player. These fixtures supply items and stock; they are not acquisition, route or
  console evidence.

Adapted from `src/dra/menu.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under
AGPL-3.0-or-later. All original definitions and tables remain private.
