# Doppleganger entrance and reward

The native BO4 service now runs its placed entry manager, paired opening plates,
light/shock geometry, eight block doors, music transitions and Life Max Up reward.
The manager holds player controls for the original opening interval, creates the
boss through its event, records a transient defeat time and waits for physical
defeat before opening the seals and presenting the reward. Ordinary travel stays
gated until durable boss records and the original transfer selection are connected.

The opposite opening plates retain the original signed movement threshold and
16-bit coordinate wrap. Only these actors bypass the native ordinary room bounds;
other actors retain the existing checks. Door patches validate their entire private
tile pair before changing collision. Public tests cover the paired triggers,
control interval, signed threshold, close/open patches, malformed patches,
contiguous effect allocation, music handoff and selected reward conversion.

The opening effect owns screen-space primitives and individual original priorities.
A bounded stable draw queue splits only effects requiring those priorities; other
actors retain their existing grouped sprite/primitive drawing. The selected
painter order places lower priorities over higher priorities. This does not claim
complete PS1 ordering-table, dithering, mask-bit or GPU presentation parity.

The shock uses the private boot image's complete SDK matrix sine/cosine pair table,
separate from the existing quarter-wave movement table. Those tables differ in
quantization and negative-angle interpretation. The selected matrix transform and
UNR perspective divider follow the pinned PSY-Z SDK implementation. No lookup,
artwork, animation, phase, tile or sound data is embedded in public sources.

The private LLDB sequence starts the original arena placements without inserting
a boss or omitting entry actors. Its 999-HP, STR-30 development character completes
the opening, selected physical combat, seal reopening, reward effect and collection.
It reaches 1,010 maximum/current HP after a level-up and the Life Max Up. This is
an explicit arena fixture, not an unmodified-character playtest, continuous castle
route or console replay. Full elemental/mist effects and audiovisual/timing parity
remain incomplete. Debug and macOS ASan/UBSan finish the same 3,601-tick
sequence with five boss-hit updates, 87 total player HP lost, 7,243 child updates
and 901 native renders. All 59 public test groups and release packaging pass.

Entry/reward algorithms adapt `bo4/doors.c`, `bo4/unk_365FC.c` and shared
`st/e_life_up.h` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
`game_matrix_trig.c` and the selected intro projection adapt
`psyz/src/psyz/libgte.c` at PSY-Z
`b52003e55c0d1b20719e984dd19bd4d760eeb14a` (MPL-2.0).
Original runtime data remains private; the existing notices ship in packages.
