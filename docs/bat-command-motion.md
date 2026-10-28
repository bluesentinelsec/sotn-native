# Bat command and special-flight motion

The core now reconstructs Wing Smash's directional command and the selected
Wing Smash/Fire of Bat controller paths. [Selected Wing Smash](player-wing-smash.md)
now connects the command to actual factories, attacks, visuals, learning and return
in gameplay. [Selected Fire of Bat](player-fire-of-bat.md) now connects its actual
factory/attack/animation/render ownership too. Echo remains unavailable. These
component comparisons cover supplied contexts independently of the live owner.

The copied controller context advances after the form's own resource gate. Original
Wing Smash input holds jump through up, up-back, back, down-back, down, down-forward
and forward, then releases jump. Each accepted direction resets twenty original
ticks; comparisons require exact direction words. Timers retain original wrapping
32-bit behavior. The US controller permits another paid Wing Smash while already
smashing. Spell MP costs come from private definitions and permit exact-cost MP,
unlike the strict transformation threshold. Learning and factory creation are
explicit requests, never manufactured allocation success.

Special motion retains original six-pixel speed, sixty-four-tick duration,
vertical steering/braking, rotation, slope precedence, wall cancellation and forced
return requests. Trail/floor/ceiling requests follow the original absolute
simulation-clock cadence. Fire retains its original animation wait, recoil and
recovery transition. Echo takes priority over Fire when both are requested; its
child behavior is separate. The caller must execute return, freeze/shake/sound,
spell learning and actual factory requests in the original order. This component
changes no player progress or runtime data and allocates no entities.

All 133 public test groups pass in Debug, Release and macOS ASan/UBSan;
release packaging passes. Authored tests cover both facings, the full command, exact/insufficient MP, US
repetition, timer expiry/wrap, missing jump, slope precedence, duration/wall return,
Fire recoil/recovery, Echo priority, disabled relics and atomic malformed-input
failures. Private muted LLDB comparisons cover 12,800 supplied post-form-gate
controller contexts against original instructions. They execute the original
command, MP, animation, speed and rotation helpers; form-finish, factory, learning,
sound and shake are explicit adapters. Ordinary-flight outputs remain outside
this special-motion comparison. No actual child combat or console replay is
implied. Debug and ASan/UBSan comparison traces are byte-identical and exit
successfully through muted LLDB. All original inputs and traces remain private.

Provenance: `dra/71830.c:CheckWingSmashInput/ControlBatForm` and
`5D5BC.c:CastSpell`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
