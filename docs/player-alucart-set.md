# Selected Alucart set and Shield rod attack

Alucart sword168 now joins Alucard sword123 in the shared [ordinary hand owner](shared-ordinary-hands.md)
through the same `w_043` program. Its record selects descriptor1 (`weaponAnims[1]`, identical in
the US data to descriptor0: frame start 0x41, ALUCARD_SWORD_SWISH on pose4, palette offset0) with
the same sixteen optional blue TILE sparkles, additive blend and SetWeaponProperties. It has no
special move: the QCF sword command stays Alucard sword123's (virtual197), and Alucart sword's
record carries no special byte. Its own attack word (2) and stats come from its actual record.

Alucart mail89 is an ordinary armor record (defense2, no stat bonuses or attributes), now accepted
in the armor slot. Alucart shield167 was already owned by the [basic shields](player-basic-shields.md).

## Set bonus

The original `IsAlucart` holds when either hand has Alucart sword168, either hand has Alucart
shield167 and the armor slot holds Alucart mail89 (`CheckEquipmentItemCount` over the hand and armor
slots; a sword in both hands is not a set). `UpdateEquipStatBonuses` then adds LCK30 to the
equipment bonus before its 99 cap, so the total luck rises by thirty, which feeds critical rates
and drops like any other luck. The native inventory header then reads "ALUCART" instead of
"ALUCARD", as the original status screen swaps its name string. The original status screen's
condition label (string45) has no native counterpart yet.

## Shield rod beside a shield

The original `CalcAttack` adds five when the weapon is Shield rod4 and the other hand's record is a
shield (category9), whichever shield it is. The native status sheet and each swing's
`GetEquipProperties` attack (which calls `CalcAttack` against `equipment[1 - hand]`) now apply that
term; earlier native builds omitted it. Hand properties therefore take the calling hand.

## Scope

The Sword Familiar126 (descriptor2, green sparkles) is not enabled: its attack adds the sword
familiar's level, and equipping it interacts with the Sword Card relic and active servant, which
have no native owner. Sun/Moonstone hour bonuses and the Vlad relics' stat bonuses in
`UpdateEquipStatBonuses` likewise remain unimplemented; those items stay unsupported. Defaults,
grants and v10 saves are unchanged. Supplied ownership and positioned-contact fixtures are not
acquisition or route evidence.

## Evidence

- Public tests: the Alucart set's LCK30 (and its absence with the mail removed or the sword in
  both hands), Shield rod's five beside Leather or Alucart shield and not beside another rod, and
  the Alucart sword record's descriptor1 check; each swing's property attack beside a shield (and
  not beside the other hand's rod); Alucart sword's runtime identity (a descriptor0 request is
  rejected, descriptor1 owns family43 with its sixteen blue tiles) in both hands. All 15 mutations of
  the set, Shield rod and Alucart sword wiring are caught.
- The private comparison runs the original `UpdateEquipStatBonuses`, both hands' `CalcAttack` and
  `GetEquipProperties` over 972 equipment/stat contexts (nine hand items, three armors, four stat
  rotations): every total, attack and critical rate matches, 1,944 property calls with their libc
  roll. Debug and ASan traces are byte-identical. The existing hand, family0, two-hand and Badelaire
  property comparisons still pass with the hand parameter.
- Native Debug/ASan protocols swing Alucart sword in either hand (descriptor1 parameter, sixteen
  blue tiles, the hand's attack destroying a zombie held on it, a positioned-contact fixture), equip
  the full set through the menus (LCK +30, gone with the sword removed) and swing Shield rod beside
  Leather shield with the boosted attack.
