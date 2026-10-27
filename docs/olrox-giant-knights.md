# Olrox's Quarters Hammer and Blade

The internal NO2 checkpoint now runs the original placed Hammer and Blade enemies,
their articulated limbs, independent weapons, physical damage, selected attack
sounds, experience and staged deaths. Both appear in original room 4. This extends
the selected development checkpoint. [Connected travel](olrox-connections.md)
now runs within its separately documented limits.

## Behavior and ownership

Hammer preserves walking, retreat, leap, turn, pain and two-handed hammer-swing
poses, including the back-arm inverse-kinematics target. Its body, fourteen limbs
and hammer retain separate positions and rotations. Blade adds its extra pelvis,
two swords, sword attacks, alternate foot attack, leap and turn logic. Its death
kneels, throws one sword, waits, scatters its limbs and finishes the body lifetime.
Sword trails advance with simulation ticks, independently of rendering.

Damage to linked body parts reaches the body's HP and experience award once;
contact cooldowns, stun and death flags propagate only through owned body parts.
Weapons keep their independent attack profiles. Fixed child reservations check
all required slots before replacement, retain generation ownership and protect
unrelated actors. Re-entry recreates matching owned survivors. Released swords
and launched death pieces can outlive their body. A departed sword cannot make
its dying body fail or allow it to write into an unrelated replacement slot.

Construction, lengths, parent indices, joint lists, angle targets, pose durations,
initializers, enemy definitions, artwork and SDK tables come from the user's
private files. Blade's shared-header data names belong to another overlay; its
NO2 metadata resolves the correct local offsets. Never embed those arrays or use
another stage's raw identities.

## Native adaptations and remaining limits

The original two-channel pose-reset loops also touch unused adjacent padding.
Native C clears only the two owned channels. Body updates use a copy of the current
actor, so relative joint zero resolves that copy instead of stale stored state.

The original hammer swing can send a negative overextension radicand to
`SquareRoot0`. Native code preserves the SDK leading-sign normalization and its
checked executable-relative lookup before the nominal square-root table. It
uses explicit integer shifts and wrapping instead of signed C overflow or an
unchecked pointer. This behavior must not be replaced with a clamped geometric
solution during authenticity work.

The original US Blade trail path writes through a null primitive pointer when
allocation fails. Native code omits that optional trail while retaining the
sword's movement and contact. The original small vertical camera shake on
Hammer's impact remains unintegrated. Full hit palettes, elemental reactions,
contact ordering, effects/audio and GPU/console presentation parity remain
incomplete. Death-piece callback replacement uses the native explosion identity;
it does not execute a function pointer retained in an empty actor slot.

Original ordinary NO2 layouts place Hammer and Blade. They do not place the
compiled Owl/Owl Knight or Gurkha families; unused overlay functions are not
proof of an ordinary encounter. Those enemies still need integration in areas
that actually place them. Spectral Sword, remaining NO2 mechanisms, Olrox,
connected progression and the complete game remain separate work.

## Validation

Public tests use authored synthetic construction, poses, collision and SDK
fixtures. They cover joint anchoring, copied-root positioning, both pose channels,
signed SDK overextension, linked damage/cooldowns, independent weapon contact,
re-entry, stale ownership, bounded death pieces, departed swords, malformed
construction/state and optional trail-pool exhaustion.

All 106 Debug, macOS AddressSanitizer/UndefinedBehaviorSanitizer and Release
test groups pass. `make package` produces the asset-free macOS archive.

Private component comparisons execute the original US instructions with isolated
initialization, collision, sound and primitive-allocation adapters. They compare
Hammer and Blade across both facings, changing player positions, injected pain
and physical-death flags: 160,284 body/joint snapshots over 10,286 simulation ticks
match, including geometric/draw rotations, attacks, hitboxes, pose cursors, RNG
and sound requests. These controlled component inputs are not an ordinary
encounter or a console replay.

Separate native LLDB checks start original room 4, protect the ordinary development
character without granting equipment/stats or enemy damage, use its Gladius to
hurt and defeat each placed enemy, verify experience and cleanup, and render the
room and death effects. Hammer awards 332 EXP at tick 677 plus 180 cleanup ticks
with 215 rendered frames; Blade awards 303 EXP at tick 1147 plus 180 cleanup ticks
with 332 rendered frames. Debug and macOS ASan/UBSan both pass these sequences.
These explicit starts and protection fixtures do not
establish balance, an uninterrupted route, whole-area or console parity. Existing
ordinary save slots remain untouched; transient joints do not change save v9.

Provenance: `st/e_hammer.h`, `st/e_blade.h`, `st/giantbro_helpers.h`,
`st/giantbro_helpers_2.h`, selected original SDK arithmetic and NO2 identities at
sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
All original runtime arrays remain private.
