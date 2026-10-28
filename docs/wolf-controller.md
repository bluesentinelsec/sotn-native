# Wolf active controller component

The core reconstructs the original US active Wolf substates: idle, walking,
turning, sprinting, crouching, jumping, falling, recoil, sliding and swimming.
It consumes copied body-root/contact/input/relic snapshots after the owning form
resource gate. It enables no live Wolf controls or relic switches. Actual entry,
return, articulated bodies, attacks, animation advancement, collision integration
and presentation remain unwired. Selected [linked body components](wolf-articulation.md)
retain their own scoped comparisons.

The owner decrements shared player timers before calling the service. The service
then decrements its wrapping run/swim windows, evaluates original swim eligibility,
runs one selected substate, copies raw contacts and applies the original run-frame
cadence and body-angle clamp. It integrates no position and invents no readiness.
[Capture bounds and jump helpers](wolf-motion.md) retain their separate scope.

Preserve the original US behavior, including these asymmetries:

- Leftward running without Power of Wolf uses the original comparison against
  positive three; it immediately caps a negative running speed to minus three.
- Wall thresholds differ between directions. Airborne sprint tests the left wall
  for both signs of horizontal velocity.
- Falling checks landing before gravity; recoil checks it after gravity.
- Jump landing has separate stand and walk tests. Standing can change the mode
  used by the second test within the same update.
- Normal direction handling gives Right priority and records actual turns;
  swimming gives Left priority and changes facing without that turn signal.
- The US charge button is the left hand (Square), not either attack button.

Eligible Skill of Wolf charge reads spell four's private MP cost, permits exact
cost, reports that cost once and requests the original learned-spell update.
The owner must supply the actual completed quarter-circle command snapshot.
Ordinary attack intent does not establish an owned hitbox or successful factory.

Factory, sound and shake requests retain relative call order and capture the
player source at that call, including temporary whole-word ceiling offsets with
fraction preservation. Wall impact reports input hold and a return request for
its owning transformation service. It does not fake that service's work or create
an unrelated entity. Invalid snapshots or missing eligible spell data fail
atomically, leaving the caller's frame and event output unchanged.

Public authored tests cover original cap/direction quirks, charge MP/ownership,
landing and gravity order, swim gates, timer wrap, whole-word request-position
wrap, input hold, platform drops and atomic failures. Private muted LLDB checks
compare 65,536 supplied active contexts across all eight supported substates,
including 433 successful charge casts, against original instructions. Original
substates, helpers, relic tests, animation changes, MP spending and spell learning
execute. Explicit factory/sound/shake adapters record requests; the forced-return
boundary reports intent without substituting a completed native transformation.
Debug and ASan/UBSan traces are identical. These component comparisons establish
no live body ownership, ordinary relic
acquisition, full Wolf movement, uninterrupted route or console parity.

Provenance: selected `dra/wolfForm.c` active controller/substates and helpers,
pinned sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`,
AGPL-3.0-or-later. Original tables, binaries and comparison traces remain private.
