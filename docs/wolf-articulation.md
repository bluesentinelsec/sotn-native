# Wolf linked body component

The core reconstructs selected original US Wolf followers in actual slots
18/19/21..30: front/rear body pieces, nine linked spine pieces and the final tail.
This is an internal component. Live Wolf controls and relic switches remain
disabled until actual root/head, transformation, attack and presentation owners
are connected.

The native owner must invoke each selected slot in original entity order,
interleaving the actual head update at slot 20. The component retains all other
parts, including supplied root 17/head 20. It never allocates entities, claims
readiness or substitutes a body for missing ownership. The owner must check the
complete generation and retire actual children when Wolf status ends.

Copied player animation, pose, angle and original private pose tables drive the
selected frames, bobbing, pivots and attachments. Preserve fixed-point fractions
and signed whole-word offsets. Turning frame substitutions remain retained when
the next substate does not overwrite them. Front/rear/tail priorities differ;
spine and rear pieces use different speed-dependent opacity caps.

Each spine's first update resets the shared wave, including later children in
the same frame. The first spine computes the shared target; subsequent spines
interpolate from that retained target and their own rate. Preserve signed
truncation for interpolation, SDK shift rounding for motion and the original two
separate rotation comparisons: a small overshoot can cancel within one update.
The tail runs the original capture-bounds service only after its own attachment.
These capture offsets are distinct from physical collision sensors.

The front body reports spell-thirteen rush attack intent above the original
speed threshold, except during the charge state. Actual hit feedback consumes the
private spell cost once, including exact-cost MP; insufficient MP requests the
original skid animation/mode without resetting the animation cursor. This report
creates no hitbox or attack power. The eventual native attack owner must perform
original initialization, collision, damage and generation-checked feedback.

Public tests use authored pose/SDK fixtures to cover linked origins, retained
frames, fractions, priority, wave initialization, rush feedback/MP, same-frame
mode changes and atomic malformed-input failures. Private muted LLDB comparisons
cover 8,192 supplied contexts and 98,304 selected follower updates, including
1,551 rush MP charges. Original pose tables, SDK trigonometry, spell MP and
capture-bounds instructions execute. An explicit attack-initialization adapter
records intent; root/head positions are supplied snapshots. Debug and
ASan/UBSan traces are identical. These checks establish no live Wolf, complete
body/attack rendering, ordinary relic acquisition, route or console parity.

Provenance: selected `dra/wolfForm.c:func_80130264`, `func_80130618`,
`func_80130E94` and `func_8013136C`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Private tables, artwork, binaries and traces remain outside public source/tests.
