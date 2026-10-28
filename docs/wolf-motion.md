# Wolf motion and capture-bounds components

The core reconstructs selected original US Wolf capture-bounds, jump and animation
cadence rules. Live Wolf controls and relic switches remain disabled pending
actual body/factory ownership, collision and presentation integration. The
[active controller component](wolf-controller.md) retains its own scoped checks.
These components allocate no entities and establish no transformation readiness.

Transformation capture bounds read copied signed whole-position words from actual
slots 17..30 at the original tail update point. Preserve the source's selected parts, min/max
order and asymmetric facing limits: the forward limit is twenty pixels in US,
and the rear limit is forty-four. Top/bottom limits remain one-sided. Outputs
are raw capture offsets, not a normalized rectangle; arbitrary supplied positions
can produce inverted offsets. These bounds feed the original Wolf BlinkWhite
capture; physical collision sensors are a separate service. The eventual owner
must preserve fractions and validate the actual body generation before supplying
its snapshot.

The jump helper runs only after the owning controller's eligibility checks.
Sprint mode takes priority over Up and selects the original facing/slope launch,
including exact slope-word matches. Ordinary directional jumps, turning mode
and swimming retain their different animation/mode choices. The helper resets
the original animation cursor, retains horizontal speed and wraps the signed
32-bit body angle before the later post-controller clamp.

Cadence reports the eight mutable run-frame durations at the original four-,
five- and six-pixel speed thresholds and clamps the body angle to its original
range. It advances no time or animation. The owner still handles private frame
words, previous-contact state, priority and the rest of the controller's writes.

Public authored tests cover both facings, ignored body parts, asymmetric and
inverted capture bounds, all sixteen slope words, sprint/Up/swim ordering, angle wrap,
speed thresholds and atomic invalid-input failures. Private muted LLDB runs
compare 4,096 supplied articulated bodies, 24,000 jump transitions and 144 cadence
contexts against original instructions. Original capture-bounds and jump functions,
including `SetPlayerAnim`, execute without service adapters. The cadence check
runs the bounded original post-controller block and compares its selected angle/
duration outputs while verifying private frame words stay unchanged.

Debug and ASan/UBSan traces are identical. Original inputs and traces remain
private. These supplied component comparisons
do not establish live Wolf, ordinary relic acquisition, complete movement or
console parity. Preserve the original left-running speed-cap quirk when porting
the remaining controller; do not silently replace it with symmetric acceleration.

Provenance: `dra/wolfForm.c:func_8012C600`, `func_8012CCE4` and selected
`PlayerStepMorphWolf` post-step instructions, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
