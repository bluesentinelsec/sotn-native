# Laboratory character progression

Defeating an integrated enemy now awards its private original EXP reward,
adjusted by the enemy/player level difference. All due levels are processed.
The native core reads the original US experience thresholds and HP growth table
at runtime. HP, maximum HP/MP/hearts, base stats, equipment calculations, and HUD
values update together. The status menu shows EXP and the remaining amount for
the next level. Level 99 stops further experience growth.

The selected original level path increases current HP by the HP gain; it does
not refill current MP or hearts. MP regeneration advances once per 64 gameplay
ticks; Mystic pendant adds the original mid-cycle recovery when equipped.
Menus stop the gameplay clock, so opening a menu cannot farm regeneration.
Save-point activation restores the current maxima rather than the old fixed
70/20 profile values. The character still starts from the explicit development
profile, not original new-game progression.

Permanent Laboratory Heart/Life Max Up pickups now increase the relevant maximum
by five with the selected original healing behavior. Dummy prize 13 remains
unsupported. Full level-up/vessel visuals, the original shared RNG consumption,
familiar EXP, and unintegrated enemy families remain incomplete. The original
level-up sound is emitted; this is not a claim of complete effects parity.

Public synthetic tests cover rewards, level adjustment, simultaneous levels,
resource changes, regeneration, vessels, level 99, malformed tables, unchanged
state/RNG after failure, and awarding EXP only once for a killed actor. A private
comparison against the unchanged pinned `CheckAndDoLevelUp` matched 2,000 cases,
including multi-level MP/stat RNG order. A targeted native LLDB encounter started
at 99 EXP, defeated original Bloody Zombies, and reached level 2 with updated
HP/MP/hearts maxima. That targeted setup is not an original stage-entry playthrough.

The adapted paths are `src/dra/5D5BC.c:CheckAndDoLevelUp`, `func_800FE044`,
`src/dra/5F60C.c` MP recovery, and `src/st/e_collect.h` vessel collection, at the
pinned reconstructed revision under AGPL-3.0-or-later. All thresholds, HP growth,
enemy definitions, artwork, and sound data remain private inputs.

Version 3 [native saves](native-saves.md) retain supported character inventory and
progression. Versions 1/2 migrate explicitly from their earlier limited profile.
