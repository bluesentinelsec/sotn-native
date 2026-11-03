# Shared ordinary hand ownership

Native empty-hand, Basilard18, Short sword19, Gladius86, Mace128, Alucard sword123
and Alucard shield16 now use the real shared slot16 owner. Normal equipment uses
factory48 and actual E0/F0 callbacks; empty hand uses factories42/43 and EE/FE.
The factory reservation precedes animation and timer changes. Exhaustion rejects
the hand use without producing an attack. Ordinary non-shield use consumes the
original casting-grunt RNG before later entity initialization and property RNG.
The shield does not request that grunt. Curse failure requests real outline57
and retains its original FF lock word.

The weapon callback reads the current player animation and full fractional
position. Private descriptor/frame/hitbox streams decide sound, sprite and
retirement. FFFF retires before hitbox writes; a negative player pose timer
retires after those writes. Native GetEquipProperties uses the character libc
cursor, actual stat/buff/poison attributes and SDK root calculation. Slot16 uses
enemy group1 or2 for its actual hand and the existing corresponding enemy
cooldown channel. Native player-side legacy attack/shield views are suppressed,
so they cannot duplicate these attacks, sprites or sounds. Raw component callers
retain their earlier frame interface and do not gain native ownership.

There is one actual slot16. A new sword or shield replaces its previous owner.
Held shield control observes the real preceding unk48 state and can raise the
shield again during permitted recovery. Normal spell entry performs the original
conditional shield retirement. Mist/Bat/Wolf entry performs its actual weapon
retirement before another owner receives the slot. These retirements commit real
stage rows after player success. No extra weapon or primitive pool is introduced.

## Alucard sword and shield

Alucard sword reserves sixteen optional real TILE rows before its property call.
At the original pose/timer gate, each tile consumes three libc calls for X, Y and
delay. Original colors, nine-update ages, shrink/drift and priority are retained.
Exhaustion omits the tiles and their RNG; the primary sprite/attack continues and
never retries that allocation. Camera translation moves existing tile coordinates
without another update. Player shove moves the PLAYER_LOCKED weapon's full fixed
position, leaving the already emitted absolute tiles in place.

The shield owns its actual private raise/hold/movement/lower animation. Ordinary
contact continues to block susceptible projectiles without enemy HP damage.
Alucard shield's displayed/GetEquipProperties attack is zero, including buffs;
the original SetWeaponProperties hit-state4 override and subsequent attack1
assignment remain distinct operations. Original per-hand cached attack is retained
for its program context. Shield-rod spell producers and their buff are unavailable.

An actual surviving hit starts shield phase4: frame3E, live player velocity,
airborne doubling, -3.5 vertical velocity, gravity20/128 and rotation128. Its signed
timer decrements twice per callback. The original final drawFlags assignment
overwrites the provisional rotate/Blink bits; the native owner preserves that
assignment rather than inventing a visual correction. Actual offscreen culling
can retire the shield before its sixty-four-callback lifetime.

The callbacks copy only the original drawFlags, scaleY and Y pivot. The selected
native normal context uses unit scale. Other transform/palette producers and
complete raster/ordering parity remain unfinished.

## Persistence and failure boundaries

Weapon37 previous-pose BSS and weapon52 cached attack belong to the actual per-hand
program. Ordinary same-family/empty-hand menus and room rebases retain them without
updates. Genuine program replacement, the selected Armor reload boundary and full
Continue restore fresh program state. They are not ordinary-save fields. Saves
remain v9; default equipment, resources and relic grants are unchanged.

Menu/map/draw/frozen holds do not update ordinary weapon callbacks or RNG. Real
stage shove and room translations add no callback or animation update. Dead-player
hit clearing disables the actual attack. The native Outer Wall elevator consumes
the original status readiness mask when shared hands are bound; it no longer
depends on a player-side attack view.

Failed player updates restore actual reservations and casting RNG before their
commit. A later selected effect/upload failure restores the real runtime, program
data, primitive leases, property/particle RNG and player snapshot at the post-player
boundary. The earlier successful player's grunt belongs to its earlier commit.
Whole-pass rollback remains an open milestone gate.

## Evidence and remaining work

Authored public tests exercise callback ordering, malformed-table atomicity,
shield knock-away, actual shared row replacement, groups1/2, optional pool
exhaustion/no retry, freeze, room translation, shove and program retention. Elevator
tests exercise observable original status gates with shared hands bound.

Private instruction comparisons cover 18,328 complete callbacks in 544 supplied
contexts, 17,480 private animation calls and 71,616 TILE snapshots; 215,040 complete
hand-helper contexts; and 22,400 complete GetEquipProperties/CalcAttack contexts.
Explicit property/resource/sprite/factory/destruction/RNG-result adapters retain
their evidence boundaries. Runtime tables and artwork remain private inputs.

Native protocols exercise both hands, real existing-enemy HP/death/EXP, actual
shield knock-away from stage contact, form-entry retirement, menu/draw holds,
sixteen late effect failures, fourteen player failures, actual pool exhaustion,
program reloads and in-memory v9 Continue. Supplied equipment/relic/protection,
position, foreign occupancy, malformed data, Talisman and save-room contexts are
not acquisition, uninterrupted route or console evidence. No user save is written.

The selected [weapon0 sword batch](player-basic-swords.md) now adds19 one-hand
items through this owner. Shield-rod spells, weapon37 secondary producers, further equipment,
familiars, full sharing/RNG/audio and Alucard/castle parity remain unfinished.
Selected [Namakura/two-hand ownership](player-two-hand-equipment.md) now runs.
Selected [Badelaire125](player-playtime-badelaire.md) now binds its actual playtime/stat owner and variant3. All six [milestone gates](alucard-status.md) remain open.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_000.c`, `w_002.c`, `w_031.c`,
`w_037.c`, `w_043.c`, `w_052.c`, `dra/6E42C.c`, `6DF70.c`, `7879C.c`,
`alu_anim.c`, `5D5BC.c:GetEquipProperties`, `menu.c:CalcAttack`, `5087C.c`
and `st/no1/unk_3B5E0.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
