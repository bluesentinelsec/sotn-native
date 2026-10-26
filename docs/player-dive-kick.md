# Selected normal-form dive kick

Alucard can now dive kick from the normal Jump state after using his second
jump. Release Jump, then hold Down and tap Jump again: arrows plus X/Space on
keyboard, or d-pad plus Xbox A. A second jump taken from the original Fall state
retains that state and does not enter the Jump-only kick path. Relic ownership
and ordinary saves follow the [Leap Stone service](player-leap-stone.md); no
development start gains a new relic automatically.

The selected path uses original animation 34, a six-pixel/tick downward launch,
directional 4.5-pixel/tick horizontal speed, ordinary drag/gravity and the attack
voice request. Its owned invisible child follows Alucard with the original
offset (9,21) and half extents (4,5). It uses empty-hand power with equipped body
attack bonuses, poison reduction and a one-time original critical roll. Hand
weapons and shields remain separate; a Gladius does not give the kick sword power.

The stage combat service damages supported enemies and breakables and delivers
the original normal/deflection/projectile result to the current child generation.
Normal-hit feedback reaches the child after the next player step; the following
player step rebounds. Held Jump gives -4.875 pixels/tick, released Jump -4.25.
The rebound suppresses the ordinary early-release cap and restores the original
jump flags, allowing another leap/kick sequence. Deflection/projectile feedback
does not rebound. Enemy cooldown survives a subsequent kick generation.

Landing, damage, script holds or another player state retire the child. Stale
feedback fails without mutation. Selected fast/hard landings retain the original
horizontal slide or half-speed crouch recovery. Kick state, generations and
feedback are transient and never enter ordinary saves; save format remains v9.

## Validation and limits

Authored public tests cover launch/direction, owned hitbox/power, critical roll
lifetime, held input, feedback delay, both rebound strengths, repeated leaps,
stale feedback, interruption, hard landing and atomic malformed-data rejection.
Character fixtures distinguish empty-hand/body bonuses from equipped weapon power.
Actor fixtures verify damage, death credit and cooldown reuse. Chapel spike-ball
tests retain their existing group checks and add simultaneous kick/group isolation.
All 109 Debug, Release and macOS ASan/UBSan test groups and asset-free release
packaging pass.

Private instruction comparisons match 3,840 ticks in 384 sequences against the
original `PlayerStepJump`, transition/jump helpers and `EntityDiveKickAttack`.
They vary movement flags, directional input, facing, substate, velocity, poison,
held Jump and hit results. Equipment calculation, factory/group allocation,
empty-room collision and sound delivery are isolated service adapters. This is
component evidence, not a full original-frame or console replay.

A muted native LLDB route uses ordinary movement/jump inputs in Laboratory room
3: 17 kicks, eight hits, four rebounds and one zombie kill over 522 simulation
ticks and 131 draws. It uses an explicit private Leap Stone grant and contact
protection, with unchanged character/enemy power and no position or enemy-hit
injection. It does not prove ordinary relic acquisition or an uninterrupted route.
Debug and macOS ASan/UBSan runs both exit normally and explicitly verify SDL dummy
audio before simulation.

The kick owns a separate native cooldown channel without moving existing hand,
projectile or Chapel-ball channels. The original shared factory capacity and
dynamic attack-group contention still need integration. Complete landing/effect
presentation, audio/timing parity, Gravity Boots, transformations, other abilities
and the rest of the castle remain incomplete. Do not call Alucard or Leap Stone
combat complete from these checks.

## Provenance

`player.c`, `player_kick.c` and selected stage feedback adapt `dra/71830.c`,
`7879C.c:EntityDiveKickAttack`, `5D5BC.c:GetEquipProperties`, and `menu.c:CalcAttack`
from [the pinned reference](../config/reference-us.json), under AGPL-3.0-or-later.
Original definitions, animation, sprites and sound remain private runtime data.
