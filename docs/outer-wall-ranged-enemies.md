# Selected Outer Wall ranged encounters

Bone Muskets and Bone Archers now run in the native Outer Wall profile. They retain
their original entity identities, private initializers, animation streams, hitboxes
and selected attack/death rules. Other Wall enemies and mechanisms remain incomplete.

Bone Muskets reserve their original three-actor group. The soldiers coordinate
volleys, retreat, reload and return using separate health and checked neighbor
references. A slot generation prevents a reused actor from receiving an old group's
signal. Shots retain the original shield-interception interval, subsequent damaging
line and selected muzzle presentation. Death signals and group cleanup remain separate
from the original independent health of each soldier.

Bone Archers approach/retreat, jump at selected obstacles, alternate original shot
recipes and stances, and emit their selected death fragments. Arrows can break or
attach briefly to Alucard. Contact notification remains independent of which enemy
wins that tick's damage selection. The original US shot-offset rule adds its private
horizontal offset for both facings; this checkpoint preserves that behavior. Attached
positions retain the original signed 16-bit interpretation.

Complete hit flashes, drops, effects, elemental/status damage, hardware line rasterization
and full timing/audio parity remain incomplete. Do not call these encounters or
Outer Wall console-parity complete.

## Validation and provenance

Public synthetic tests exercise coordinated volleys and movement, shot blocking,
independent group death/cleanup, invalid neighbor offsets and indices, Archer recipes,
fragment lifetimes, arrow attachment despite a stronger simultaneous contact, and
signed coordinate wrapping. No original game tables or artwork enter these fixtures.

Private native LLDB fixtures render each family on its original platform, exercise
selected attacks/projectiles and forced death/cleanup, and use a protected development
player. They start in explicit rooms and do not establish an ordinary combat playthrough,
continuous route or console replay. Dummy audio validates decoding, not listening.

Adapted from `st/e_bone_musket.h`, `st/e_bone_archer.h` and selected shared collision
rules at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under
AGPL-3.0-or-later. Original recipes, hitboxes, motion, animations, graphics and audio
remain in ignored user-supplied files.
