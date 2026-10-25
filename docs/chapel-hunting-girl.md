# Chapel Hunting Girl

The internal [Chapel checkpoint](chapel-runtime.md) now runs selected original
Hunting Girl spirit/sword presentation, approach/retreat, all three attack combos,
three blade hitboxes, hit recovery and death. Connected Chapel travel, remaining
encounters, tower mechanisms, Maria and boss progression remain incomplete.

The sword grounds through its private sensors and remembers its original room
height. Transitions restore that height one pixel per tick, then mirror its angle
when changing facing. The bat-status branch follows player height instead. Idle,
recovery and attack spirits use five private rectangles, original sine-driven
brightness, additive presentation and fade timing, including the last zero-color
frame. Approach/retreat waits and the long recovery cycle retain their original
ordering and distance thresholds.

Attack motion reads private fixed-point deltas, angles, durations and pattern
pointers. Thirty-one owned line primitives retain the original screen-space sword
trail, endpoint colors, fade rates and sound timing. Three adjacent owned children
follow quarter-points along the rotated blade; each deals contact damage. Their
power doubles for bat/wolf status, as in the reference. Child initialization
preflights all reservations and generation capacity before replacing any child;
unrelated placements stay protected and stale owners retire their children.

A nonlethal hit interrupts the combo, shows its selected spirit and applies the
original knockback, friction and recovery. Death retires the blade children,
shows the selected spirit, then drops the sword with intermittent original flames
until offscreen cleanup. Optional exhausted primitive/effect pools follow the
selected original omission behavior; invalid data/state/ownership fails explicitly.
All original profiles, animations, motion, artwork and SDK tables remain private.
Transient actors do not change the ordinary v8 save format.

## Required C adaptation

The reconstructed attack routine writes through a null trail pointer during
non-attacking movement and can reach zero-duration terminators before division.
Native code omits writes through the absent trail, retains the subsequent END/
cursor processing, validates durations before division and uses defined signed
multiplication/shifts. This differs from the comparison backend's null-pointer
early return, which also bypasses END processing. Synthetic loops verify the
chosen defined control flow. PS1 low-memory side effects are not reproduced;
console replay comparison of these paths remains outstanding. This adaptation
must not be described as proven console parity or an optional gameplay enhancement.

## Validation

All 86 public groups pass in Debug and macOS ASan/UBSan; release packaging passes.
Synthetic cases cover reservation/re-entry/orphan safety, primitive/generation
exhaustion, blade power, private geometry/sine/pointer data, repeated non-attacking
loops, additive spirit and screen-space trail fading, attack-cycle transitions,
hit recovery, original death/flame behavior and offscreen ownership cleanup.

A private LLDB fixture starts the original placed room-13 actor with a protected
999-HP character. It observes completion of all three combos, positions blade-only
contact (18 HP), uses an ordinary low-strength attack for a nonlethal hit, observes
recovery, then applies a high-strength fixture to finish the ordinary attack kill.
Death flames and all three child retirements are verified. Debug and sanitizer
runs each finish at tick 776 with 195 rendered frames and successful exit. Captured
spirit/trail frames were visually inspected. Forced contact/target positioning and
protection isolate components; this is not ordinary traversal, an uninterrupted
encounter replay or console audiovisual/timing parity evidence. Dummy audio
validates decoding/ownership rather than listening or PCM comparison.
