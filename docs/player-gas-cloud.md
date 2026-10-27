# Selected Gas Cloud combat

Owned, enabled Gas Cloud now adds the original invisible damage children when
Alucard enters [Mist](player-mist.md). Mist and Power of Mist remain separate
relic gates. Gas Cloud can be switched through the relic panel; ordinary starts
grant none of these relics.

## Original rules

The private blueprint requests seventeen children in original slots 32 through
47. Its noncritical factory stops when allocation fails: an empty sixteen-slot
range produces sixteen children, not seventeen. The component accepts an available
slot mask and preserves lowest-free allocation and sequential child parameters.
It consumes one spell-initialization RNG value for each allocated child, including
children that never contact an enemy.

Each child reads private spell definition twelve. Power includes original INT and
random adjustment and Mojo Mail's subsequent half-power bonus. Mojo Mail's ordinary
equipment record is now supported. Element, stun, hit effect and invulnerability
duration come from the private spell metadata; no game tables are embedded.

Child zero uses a 28-pixel half-extent box at the midpoint of Alucard and his real
Mist entity. Other children use eight-pixel half extents at successive real cloud
ring points. Negative midpoint division truncates toward zero. Positions follow
the actual visual after its update and refresh on room rebases without advancing
the effect a second time. Children persist through restoration while Mist status
remains set; normal return and full stage initialization retire them.

All children share original enemy-invulnerability group four. An overlapping ring
therefore cannot multiply damage during one cooldown. New Mist generations retain
an enemy's existing cooldown. Native combat applies ordinary elemental damage,
linked enemy hit propagation, death, EXP and pickups. Disabling the relic suspends
existing hitboxes; enabling it resumes them. Enabling Gas Cloud midway through a
Mist transformation does not create a factory that was absent at entry.

## Validation and remaining work

All 130 test groups pass in Debug, Release and macOS ASan/UBSan;
`make package` passes. Authored public fixtures cover allocation bounds, RNG consumption, spell modifiers,
real effect ownership, negative midpoint rounding, enable/disable, retirement and
failed-input atomicity. Actor fixtures cover overlapping children, retained
cooldown across generations, disabled contact and subsequent enemy death.

A private comparison executes the original factory scans, spell calculation,
group allocation and child controllers across 384 contexts and 3,072 child
snapshots. It adapts BIOS RNG, equipment-count lookup and destruction and supplies
effect-point snapshots. Those checks establish selected component agreement;
they do not independently validate the whole effect or PS1 collision/rendering.
Private runtime inputs and traces remain ignored. Debug/sanitizer traces agree.

Muted LLDB checks of both actual CMake app builds enter Gas Cloud with explicit
Mist/Power/Gas relic grants, kill two placed Laboratory zombies, award 54 EXP and
spend 18 MP before natural restoration. No health, MP, stats, position or enemy
state is injected; ordinary development stats remain in use. Native-session traces
match between Debug and ASan/UBSan, including simulation, owned RNG streams and
drawn frames. A separate explicit connector start crosses a room with live gas
children, then an isolated save-room transfer/UI write and fresh byte-identical
version-nine load retain all three relics and retire transient children. Existing
user slots remain untouched. These fixtures do not establish ordinary acquisition,
uninterrupted traversal or console parity.

The session now exchanges actual slot masks with the [selected Bat runtime](player-wing-smash.md),
retaining each allocated slot even while its hitbox is disabled. Shared original
factory/entity contention with every projectile, familiar and spell remains
incomplete, as does interleaving all original attack groups and entity update
order. Gas Cloud uses one dedicated native cooldown channel for its original group;
it does not yet share a global allocator with unintegrated abilities. Complete
Mist raster/audio/timing and ordinary acquisition still need work. This feature
does not make Alucard, the castle or the game complete.

Provenance: `dra/7E4BC.c:UnknownEntId48`, selected `7879C.c:EntityFactory`,
`func_8011A328/func_80118894` and `5D5BC.c:GetSpellDef`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
