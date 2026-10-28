# Selected playable Fire of Bat

Owned, enabled Soul of Bat and Fire of Bat now connect hand-attack input during
ordinary flight to the original casting animation, spell MP cost, recoil and
recovery. Z/D or Xbox X/B fires. The relic panel can switch an owned Fire of Bat;
ordinary starts grant no relics. [Selected Echo](player-echo-of-bat.md) now runs; Wolf remains unavailable.

The command runs after Bat's own resource gate. Casting accepts the original exact
spell cost; transformation eligibility still uses its strict threshold. A paid
cast can end before projectile birth if Bat's later resource check forces return.
Recovery must reach the original C4 pose-eight condition before another cast.
Relic switching prevents new casts without erasing already-owned projectiles.

## Ownership, combat and presentation

Actual factory slots and private blueprint 81 select original child 45 in slots
32..47, sharing availability with Gas Cloud. Optional exhausted factories create
no invented shot and spend no projectile RNG. Successful birth consumes the
original spell-nine initialization, INT/RNG/Mojo Mail adjustment and attack-group
selection 7..10. Original element, critical rate, stun, invincibility and hitbox
metadata feed native enemy/candle damage, death and EXP. Group cooldown survives
child replacement; independent groups retain separate cooldowns. Feedback targets
the observed generation and retires the projectile on its next update.

The [motion component](bat-fireball-motion.md) retains launch fractions, speed,
facing, growth and impact ordering. Actual private bank-nine animation loops after
the child update. Rendering reads private multipart artwork/palettes and applies
SDK signed-floor scaling, including negative pivots. Existing actor transform
paths keep their own behavior. The real parameter-five BlinkWhite flash and
original shot sound now run. Birth sets original subweapon-use timer ten to four;
it grants no invincibility. Shots can outlive Bat form, retire offscreen, and
rebase across rooms without extra simulation. Full stage initialization retires
them. Ordinary version-nine saves retain active/disabled relics, not projectiles.

## Evidence and remaining scope

All 137 public test groups pass in Debug, Release and macOS ASan/UBSan; packaging
passes. Authored fixtures cover owned/disabled command gates, animation/recovery,
actual slot/group/attack initialization, shot persistence, generation-checked hit
feedback, group cooldown/death, rebases without extra ticks, and SDK negative-pivot
scaling. Other relic-switch and ordinary Bat/Wing checks remain regression coverage.

Private original-instruction comparisons cover 240 supplied factory/capacity
contexts, 192 births and 2,370 projectile/spell/group/animation/impact records.
Original constructor, factory, SetSpeedX, spell/group and UpdateAnim instructions
execute; RNG, equipment, sound and destruction use explicit adapters. These
controlled snapshots do not prove a console replay. Debug/sanitizer traces match.
Earlier motion-only comparisons retain their narrower documented evidence.

Muted LLDB checks of CMake libraries and the actual executable exercise casting,
recovery, private artwork, placed candle impact/retirement and MP-forced return
with ordinary development HP/MP/stats/position and explicit Bat/Fire relic grants.
A separate repeated-combat fixture grants 100 maximum/current MP to sustain four
shots and verifies placed enemy damage/death/EXP. Its HP/stats/position/enemy states
remain ordinary. These fixtures do not establish ordinary acquisition or an
uninterrupted route. An isolated save-room transfer exercises its actual
slot-fifteen UI and fresh byte-identical version-nine load, retaining Bat and a
found-but-disabled Fire relic
while retiring transient owners. Existing user saves remain untouched.

Global factory/primitive/attack-group contention with legacy subweapons and other
abilities, afterimages, complete effects/audio, water-volume coverage and full
raster/timing/route parity remain incomplete. Selected ability integration does
not make Bat, Alucard, the castle or the game complete.

Provenance: selected `dra/71830.c:ControlBatForm`, `84B88.c:EntityBatFireball`,
`7879C.c:EntityFactory/func_8011A328/func_80118894/EntityPlayerBlinkWhite`,
`alu_anim.c:UpdateAnim`, `5D5BC.c:GetSpellDef` and `4B758.c` SDK scale presentation,
pinned sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
