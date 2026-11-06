# Selected Sun/Moonstone and Vlad relic stat bonuses

`UpdateEquipStatBonuses` (menu.c) and `CalcDefense` now include the two remaining time- and
relic-dependent stat terms. Defaults, grants and saves are unchanged.

## Sunstone and Moonstone

The hour is `g_Status.timerHours % 24`, with C's signed remainder: a negative hours word gives a
negative hour, which always belongs to the Moonstone.

- Hours 6..17: each Sunstone (accessory 59) in an accessory slot adds five to every stat.
- Other hours: each Moonstone (accessory 58) does so instead.

Two copies add ten. The bonus joins the other equipment bonuses before the 99 cap and the CON×8
and INT×4 totals.

Both stones read the session's actual playtime owner. Like Badelaire, they are supported only
once that owner is bound, so a character without a clock refuses them. The native character
recalculates whenever the hour changes. The original refreshes these words lazily (every
`GetEquipProperties`, damage, menu and consumption pass), so the two differ only in stats that no
consumer reads before the next refresh.

## Vlad relics

Each active relic (`RELIC_FLAG_ACTIVE`) adds its bonus:

| Relic | Index | Effect |
| --- | --- | --- |
| Heart of Vlad | 25 | curse immunity (CalcDefense) |
| Tooth of Vlad | 26 | STR +10 |
| Rib of Vlad | 27 | CON +10 |
| Ring of Vlad | 28 | INT +10 |
| Eye of Vlad | 29 | LCK +10 |

Stat bonuses join the others before the cap. Found-but-inactive relics add nothing. The session
syncs the world's active bits into the character on every update (`ApplyCharacter`), so
pickups, shop purchases and Continue take effect at once. Curse immunity follows the existing
immunity path, so a curse contact gets the original immunity response.

No ordinary producer of the Vlad relics exists yet (their bosses are not integrated), and this
change grants none.

## Evidence

- Public tests cover the playtime gate, both stone windows (including hours 17, 18, a wrapped
  hour and a negative hour), one and two copies, each Vlad relic's stat and the Heart's immunity,
  and a malformed relic mask.
- A private MIPS oracle runs DRA's real `make_all` (UpdateEquipStatBonuses, make_att and
  CalcDefense) on 4,000 sampled contexts, varying accessories, hours, Vlad masks, the Alucart set
  and base stats. It compares every equipment bonus and total, both hands' attack and curse
  immunity. Debug and ASan traces are byte-identical, and the oracle catches a deliberate
  mutation.
- A mutation sweep catches all twelve mutants of the new terms.
- A private native LLDB probe in a real Laboratory session shows the session clock gating an
  equipped Sunstone (+5 by day, none at night). The world's relic bits add STR for an active
  Tooth, nothing for a found-only Rib, and curse immunity for the Heart. These fixtures supply
  items, relics and the clock; they are not acquisition, route or console evidence.

Adapted from `src/dra/menu.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under
AGPL-3.0-or-later. All original definitions and tables remain private.
