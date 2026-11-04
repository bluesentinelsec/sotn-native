# Selected Holy sword and weapon secondaries

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts Holy sword115
through its own weapon55 program, together with its QCF [weapon special](player-weapon-specials.md)
196. The special also needed the first native owner for a *weapon secondary*: the child that a
resident hand program's `func_ptr_80170004` runs after a blueprint56 (`WFACTORY(0x38)`)
request. [Rapier and Were bane](player-lunge-weapons.md) now reuse it; later specials can too.

Owned selection uses the ordinary inventory menu and either hand. Definitions validate the
actual category1, family55, kind1/28, animation0x41, blueprint48, variant, lock13, chain0x80,
special byte196 and special MP5. The definition reader now validates a per-item MP word
instead of rejecting any nonzero MP outside medicines. Defaults, grants and v9 saves are
unchanged. No new weapon, effect or primitive pool is introduced.

## Attack callback

Weapon55 runs the sword template with additive blending (descriptors0/1 share frameStart0x41
and sound0x60B). At descriptor1's birth it plays SFX_MAGIC_SWITCH (0x69D) and requests
`WFACTORY(0x38,0)` from the weapon's own position before `SetWeaponProperties`. A descriptor
sound in the same update follows the birth sound. Factory exhaustion silently omits the
secondary, as `CreateEntFactoryFromEntity` returning NULL does.

## Special196 player state

Special196 is kind28. A completed grounded QCF selects it when five MP are available; MP is
charged after the actual factory, and otherwise the ordinary attack follows. Kind28 sets the
player standing with zero velocity, holds `D_80139824`=10 and plays SFX_VO_ALU_ATTACK_B
(0x6EF) instead of the casting grunt and its RNG. Substep0x5C (the shared 0x53..0x56/0x5C
handler in `dra/71830.c`) keeps pose0 for ten updates with no crouch window, then releases
the attack lock at pose13 like other kinds.

## Weapon secondary owner

The factory runs on the next pass and spawns E1/F1 in the first free slot17..31. The child
copies the player's position, facing, draw flags, Y scale and pivot every callback and retires
when the player's animation leaves the descriptor's seven-frame window or unk46 clears. Its
parameter selects the descriptor whose frameStart it tests.

Holy sword's secondary reserves 64 mandatory LINE_G2 rows from the real shared stage pool
before its first callback. Exhaustion retires the child before any row write, RNG or property
roll, without retry. Birth consumes two rand() calls per row (angle, length; growth is half the
length), sets bank0x11 (+2 per hand), palette0x117 (+24), texture0x65 (+2), flags0x960000,
priority player+4, rows player+2 and additive blending. `SetWeaponProperties` uses the raw
slot16 parent's current equip id; it increments the least-used group before the original
enemyId copy from that parent, so the secondary shares the hand weapon's group, channel and
cooldown. Hitboxes start 32x32 at (12,-28) and shrink to 12x12 at (13,-28) on frame1/timer1.
Each callback decays row colors (red/green 12, blue 24, down to 17), places every row from
the screen origin and the SDK quarter-wave trig, and grows its length.

After the callback and the ordinary off-screen cull, flag0x100000 runs the original
`UpdateAnim` on the private five-entry timeline; the END control leaves timer-1 and the next
callback retires the child before touching its rows. Retirement releases all 64 rows.

## Rendering, holds and failures

The rows draw as semi-transparent LINE_G2 with drawMode0x331, whose DR_MODE selects ABR1
(additive); their first endpoints stay black. The sprite uses weapon55's SetSpriteBank2 table
(0x8017B160). Its definitions precede the table, so the renderer bounds it explicitly at five
entries instead of inferring the length. Draws apply the current shake once and never tick.
Frozen updates hold the child. Room entry translates its position and both row endpoints
without a callback; player shoves move only the entity. Death clears its hitbox. Menu weapon
changes, Wolf replacement and form retirement clear its attack mask, and the commit releases
its rows. Late failures leave the runtime, rows and character RNG unchanged.

## Evidence

Public tests cover the attack birth sound/factory/order and both hands; secondary birth with
exact RNG, rows, bank/palette/texture/priority, hitbox shrink and the ten-callback
`UpdateAnim` retirement; exhaustion without RNG; animation-window and unk46 retirement;
parameter-selected frameStart; hit type4; failure atomicity; runtime factory/slot/rows/group,
holds, rebases, shoves and row release; kind28 MP/grunt/hold and the four-MP fallback;
renderer table bounds and draw kinds. Mutations of the color decay, hitbox shrink, allocation
gate, group copy, row rebase, sound order, factory hand, grunt omission, hold, velocity,
sprite table, table bound and special MP are caught.

The private original-instruction comparison runs 7,224 complete attack callbacks (448
contexts) and 6,608 complete secondary callbacks (896 contexts: both parameters, hands,
facings, hit types, cameras, exhaustion and retirement), with the real DRA UpdateUnarmedAnim
and UpdateAnim and SLUS rand/rcos/rsin, including 365,568 row comparisons. Debug and ASan
traces are byte-identical, and a deliberate color mutation is caught. The compiled palette
loader matches the original full384-word copy for both hands. Native Debug/ASan protocols
exercise the ordinary attack, the real QCF special with MP, the pose0 hold, the real secondary
in slot17 with 64 shared rows, additive line and sprite pixels, secondary-only enemy damage,
repeated-draw holds and row release, plus the four-MP fallback, in both hands. Supplied stock,
protection, position and MP contexts are not acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_055.c`, `weapon/shared.h`, `dra/6E42C.c`,
`dra/71830.c`, `dra/7879C.c` and `dra/alu_anim.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
