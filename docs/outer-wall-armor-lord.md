# Outer Wall Armor Lord

The selected Wall encounter now runs approach/retreat, the overhead sword slide,
charged fire attack, branching floor fire wave, reactive shield, sword shadow and
staged death. Its shield reacts to the original four-tick weapon-startup flag,
not the distinct subweapon-use flag; Spear Guard observes both supported signals.
Separate sword and wave hitboxes retain their private attack values.
Original animation, decision, collision and hitbox tables are read from the user's
NO1 overlay. Optional effect allocations retain bounded, nonfatal fallbacks.

The defeated flag belongs to the loaded NO1 actor service, matching the original
overlay's static state. It survives room re-entry and resets when the overlay is
recreated. It is deliberately not converted into a permanent castle/save flag.
Sword ownership and fire-wave damage actors validate slot, owner generation and
primitive-pair generation; an expired child cannot change a reused effect's latch.

Contact now retains its element word through the native scene. An equipment
prepass applies weakness, resistance, immunity and absorption in the original
order before the existing defense/physical reaction path. Absorption restores HP
up to the maximum. This is not complete player damage integration: ailments,
special-item reactions, fire/other elemental hit and death presentation, and the
original contact-priority rules remain incomplete.

The shield uses the selected original growth/fade timing. Death retains the
selected phase timing, silhouette fade, collapsing ribbons and final particles.
Its native ribbon projection and flat lighting are bounded
presentation adaptations. Original per-vertex GTE lighting, depth ordering,
camera shake, complete blending and audiovisual/timing parity remain unfinished;
do not describe the death effect or encounter as console-matched.

Public synthetic regressions cover attack selection, sword offsets/ownership,
wave hitbox initialization, smoke/cleanup, same-overlay defeat suppression,
malformed metadata and exhausted pools. Equipment-element regressions cover
ordering, odd rounding, immunity precedence, absorption and unchanged failed outputs.
A private LLDB fixture uses the ordinary private room with a protected player,
forces a shield and lethal enemy flag, and exercises sword/fire/wave/shadow,
death cleanup and re-entry in Debug and macOS ASan/UBSan. This is an explicit
component fixture, not an ordinary-input playthrough or console parity replay.

Adapted from `src/st/e_armor_lord.h`, selected `src/st/prim_helpers.h` behavior and
`src/dra/5D5BC.c:HandleDamage` at sotn-decomp revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under AGPL-3.0-or-later.
Original game tables, assets and private validation artifacts are not distributed.
