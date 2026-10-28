# Owned Wolf player core

The selected player core now connects original Wolf entry, active controller,
physical collision and return to generation-owned factories, BlinkWhite effects
and the linked body owner. [Selected playable Wolf](player-wolf.md) now binds
these interfaces in the native scene, with owned controls and relic switching.
BlinkWhite uses its actual factory-selected slot (the selected private fixture uses
slot63), not the weapon/Mist slot16. Interleave its update with the body by that
actual slot; running all flashes before body17..30 would use stale bounds.
Ordinary starts grant no Wolf relics and saves remain version nine.

Entry checks actual owned/enabled Soul of Wolf and original MP/lockout gates.
Factory exhaustion retains substate zero. Actual factory success requests the
original body replacement and Blink23; the effect's real phase13 and finishing25
supply signals two/three. Mist entry waits for the actual Mist child before its
finishing flash. No elapsed-time readiness substitute is used. The body controller
commits only after physics, with its current generation, origin, clock and one
commit per player update; stale/repeated feedback fails without mutation.

Wolf always uses the original eight collision passes, including slow motion.
Active sensors use original crouch geometry; entry and return retain gradual sensor
restoration. Selected run animation substitutes the eight original mutable cadence
words without changing borrowed DRA bytes. QCF input retains captured facing,
signed timer-word wrap, exact directional words and the original completion hold.
Normal Wolf return preserves the original jump/flags and 24-tick lockout while
omitting Bat/Mist's separate three-pixel position adjustment. Original timer14
protects return and movement above three pixels per tick.

Preserve the original unchecked return-factory failure: sensors restore, but no
new child/generation exists and no signal three is manufactured. The explicit
missing-child state waits, matching that original allocation corner. Entry failure
retries instead. Stage initialization/Continue still restores ordinary Alucard.

Public authored tests cover relic/resource gates, missing factories, real pending
factory contexts, input timing, stale contexts, return geometry, collision-pass
quantization, forced original slot replacement and animation-data immutability.
Private muted LLDB QCF comparisons cover 2,560 direction/facing/signed-timer/
completion/cramped contexts against original instructions without adapters; Debug
and ASan/UBSan traces match. A separate manually orchestrated session fixture runs
this repository's actual player, shared factories/Blink, linked bodies and stage
primitive ownership through entry, steering/physics, MP, return and cleanup. It
uses the ordinary Laboratory character with an explicit Soul of Wolf grant and
writes no saves. Its duplicate-feedback check rejects a second body commit.

The normal-entry fixture is not a native-app controls playtest, continuous route
or console replay. Selected native scene/combat/render/factory owners now consume
these interfaces. [Selected cross-form conversions](player-wolf-cross-forms.md)
now run. Actual water-depth/swimming,
complete shared ownership and full parity remain incomplete. Standalone component
reports confer no ownership by themselves. Do not call Wolf, Alucard or the game complete.

Provenance: selected `dra/{692E8,wolfForm,704D0,alu_anim}.c` and original return
helpers, pinned sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Original artwork, tables, runtime contexts and traces remain private inputs.
