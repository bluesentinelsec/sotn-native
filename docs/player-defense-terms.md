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

## Walk armor

Walk armor (body item 19) adds `g_RoomCount / 60` to defense before the DEF buff and the clamp.
`g_RoomCount` counts newly explored map cells: `func_800F2014` sets a cell's even bit and
increments it, and the original saves it as `nRoomsExplored`. The native world already keeps
this counter (`SotnWorldState.rooms`, saved and checked against the explored bits).
`ApplyCharacter` syncs it into the character every update, so defense rises as rooms are
explored. The armor's record also lowers INT by five.

## Additive body records

Lightning mail (11), Ice mail (12), Dark armor (16), Holy mail (18), Ring of Varda (74), Ring of
Feanor (81) and the Covenant stone (86) are now supported. Their records carry only:

- defense;
- element weakness, resistance and immunity;
- stat bonuses;
- accessory attack.

The existing owner already reads all of these. The original references none of them in code (DRA
or stages), so equipping them changes stats and elements only.

A second batch is now supported on the same basis: Fire mail (10), Mirror cuirass (13),
Dracula tunic (23), Holy glasses (34), Circlet (37) and Ring of Pales (62). DRA references these
only in debug inventory grants and Library shop tables. The one exception is the Holy glasses,
which BO6 reads during the Richter fight, a stage not yet integrated. In the native port they
therefore change stats and elements only. Mirror cuirass's stone immunity takes the same US
prepass as the Medusa shield.

[Secret boots](player-secret-boots.md) now have their sprite-scale owner and the
[Ring of Arcana](enemy-drops.md#ring-of-arcana) its enemy-drop owner.

## Evidence

- Public tests cover Shield rod's +2 beside a shield, each shield immunity (and its absence with
  the other shield), Ice mail's resistance and the Ring of Varda's bonuses.
- A private MIPS oracle runs DRA's real `make_all` on 4,000 sampled contexts: shields, Shield
  rod pairs, the new armors and accessories, the Alucart set, hours, Vlad masks and base stats.
  It matches every stat bonus and total, both hands' attack, defense and all four element masks;
  the contexts also sample Walk armor with `g_RoomCount` up to 8192, the second additive batch's
  armors and head items, and every supported accessory.
  Debug and ASan traces are byte-identical. The oracle found the missing Fire shield immunity
  and catches a deliberate mutation of the Shield rod term.
- A mutation sweep catches all nine mutants of the new terms, and all four Walk armor mutants.
- A private native LLDB probe in a real Laboratory session equips the shields through the actual
  equipment menu. The player's attributes gain stone immunity from the Medusa shield and fire
  immunity from the Fire shield. Shield rod and Ice mail raise defense, and Ice mail's resistance
  reaches the player. A second probe shows the session's explored-room counter raising Walk
  armor's defense by rooms / 60, and Mirror cuirass alone gives stone immunity. These fixtures supply items, stock and explored rooms; they are
  not acquisition, route or console evidence.

Adapted from `src/dra/menu.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under
AGPL-3.0-or-later. All original definitions and tables remain private.
