# Selected enemy item drops

Killed enemies now drop items as in the original: hearts, money, their uncommon and rare items,
and the Jewel sword's gems. The [Ring of Arcana](#ring-of-arcana) (body item 75) is now
supported. Defaults, grants and save formats are unchanged; the rare-drop records use the
existing castle-flag bytes.

## Drop roll

`st/collision.h` rolls the drop after a kill's experience and kill count:

- **Eligibility.** The killed root's flags `0x400`/`0x800` select `g_testCollLuckCutoff[1..3]`
  (0x40, 0x20, 0x10). Enemies without them never drop.
- **Luck cutoff.** `rand() & 0xFF` must be below `func_800FF460`'s cutoff:
  `cutoff + cutoff * LCK / 0x80` (unsigned).
- **Slot.** `AllocEntity` takes the first free slot in 160..191. With none free, nothing more is
  rolled.
- **Item.**
  - The Jewel sword (attacker `hitEffect` 5) picks a gem: the first `g_jewelSwordDropTable`
    threshold reaching `rand() & 0xFFF`.
  - Otherwise `func_800FF494` chooses the kind:
    - **Rare.** Roll `rand() & 0xFF`, subtract `(LCK + (rand() & 0x1F)) / 20`, and for each Ring of
      Arcana copy subtract the rare rate again. If the result is below the rare rate, the enemy's
      rare item drops.
    - **Uncommon.** Otherwise subtract the rare rate, the uncommon rate once per Arcana copy and a
      second luck term. If the result is below the uncommon rate, the uncommon item drops.
    - **Prize.** Otherwise index `g_testCollPrizeTable` with `rand() % 28`, plus one for each zero
      rate and each Arcana copy.
  - **Paranthropus.** A rare Ring of Varda becomes a Turquoise until the game is cleared.
  - **Rare-drop record.** Any other rare item records the enemy's rare-drop flag from
    `g_testCollEnemyLookup`.
- **Birth.** The drop is born at the hit part with `velocityY -3.5`, so it pops up before falling:
  - items from `0x80` up use the equipment drop (raw ID 0xA, the item minus 0x80);
  - lower items use the prize (raw ID 3).

All of these `rand()` calls use the native libc combat cursor, after the hit's own damage rolls.

## Records

When a prize or equipment drop clears its first collision check, it sets its enemy-list flag in
`g_CastleFlags` bytes 0x1B0 onward. Ordinary saves already keep those bytes.

## Ring of Arcana

Each Ring of Arcana in an accessory slot lowers both item thresholds by the enemy's rate again
and moves the prize index by one, as above.

## Tables

Every integrated overlay keeps its own file-static copies of the drop tables:

- Laboratory, Entrance (first and later visit), Marble Gallery, Outer Wall, Long Library, Royal
  Chapel and Olrox's Quarters;
- the BO0, BO4 and BO5 boss overlays.

The native owner reads them at each overlay's `g_testCollLuckCutoff` and checks the original
luck/coordinate pattern and the final gem threshold first. The enemy lookup lies 0x320 bytes
before the luck cutoffs, the prize table 0x34 after and the gem table 0x74 after.

## Validation

- **Public tests.**
  - The luck scaling and the kind rolls: rare, uncommon, prize index with Arcana, and the exact
    rand calls.
  - Table validation and gem picks.
  - The actor drop: Turquoise/Varda with the record, uncommon, prize, Jewel sword.
  - Failed cutoffs and full slots spending one roll; unflagged kills; a flagged root without a
    definition failing; Ring of Arcana shifting the prize.
- **Private emulator comparison.** The original `func_800FF460` and `func_800FF494` on the actual
  DRA/SLUS match 20,000 sampled rolls (3,849 rare, 4,630 uncommon) and 67,672 rand calls. Three
  native mutants fail it.
- **Private LLDB probe.** It spawns Bloody Zombies ahead of the player in Laboratory room 3 and
  kills them with ordinary slashes:
  - 12 kills produce 6 drops: 3 prizes and 3 equipment items, including the zombie's rare item;
  - every drop is born with `velocityY -3.5`, rises, and then rests or is collected;
  - the rare drop's enemy-list record is set.

These use explicit spawn/position/one-HP/protection fixtures, not acquisition, route or console
evidence.

Adapted source: `src/st/collision.h`, `src/st/e_collect.h` and `src/dra/5D5BC.c` at the pinned
reconstructed revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
