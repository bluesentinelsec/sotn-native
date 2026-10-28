# Selected Gravity Boots ceiling effects

Gravity Boots ceiling hits now connect original `CreateCeilingHitEffect(3)`
requests to the native shared factory/child/primitive owner and camera service.
Both early and late high-jump ceilings retain their existing motion, stop,
sound, facing and recovery behavior. The selected original ceiling tile particles
and small vertical shake now run alongside them.

Capture the factory source before the later facing flip: X shifts three pixels
in the original facing direction and Y shifts up twenty-two pixels, retaining
fixed-point fractions and word wrap. The one-update player signal is an intent,
not an allocated child. Room changes validate and rebase its room-space source
without another tick; later camera follow must not replace or shift that origin
a second time. The signal clears on the next player simulation update.

The existing shared runtime reserves actual factory slots 8..15, requests original
blueprint 4/parameter 1, allocates real entity-five children, and commits actual stage
rows including the particle sentinel. It uses private profiles and the shared RNG.
A full optional factory/primitive pool omits the visual without changing jump
motion or manufacturing readiness; failed row allocation consumes no particle RNG.
Late malformed profiles roll back the runtime and character snapshot. The scene
queues shake 3 once and the original impact sound keeps its existing owner.
No second factory, child, primitive or RNG pool is introduced.

Public fixtures check pre-flip source coordinates, early/late ceiling motion,
one-update reset, reversible no-tick room transfers, actual shared birth/source/
parameter/row ownership, six RNG calls for an authored three-particle profile,
nonzero-camera projection, rebase without respawning, allocation omission and
atomic malformed-data failure. The public profile is authored, not copied game data.

Private muted LLDB comparisons match 15,840 supplied body/transition/helper
instruction contexts, including both facings and signed timer wrap. Original
ceiling-helper instructions capture the factory origin/facing/priority and execute
the real shake request; factory and sound calls use explicit boundary adapters.
Debug and ASan traces match. This does not execute the complete original frame.

Private native Step/Draw checks use Down/Up/Jump against an actual Laboratory
ceiling, with only an explicit Gravity Boots relic grant. They retain ordinary
HP/MP/stats/enemies and use no position, protection or contact fixture. They check
MP 4, real dust/sentinel rows, captured source, shake 3, rendering and transient
cleanup. They write no saves and do not establish ordinary relic acquisition,
whole-ability, route or console parity. Separate checks of the CMake executable
confirm those commands, dust rows/source, shake and an ordinary SDL quit at
190 ticks in Debug and ASan/UBSan. Runtime inputs and traces remain private.

All 152 tests pass in Debug, Release and macOS ASan/UBSan, alongside 34 Python
checks and release packaging.

Full GPU/raster/dithering parity, original shared contention across every ability,
other impact producers and the remaining Alucard systems are incomplete. Ordinary
starts grant no relics; application/save versions and existing user slots remain
unchanged. Do not call Alucard, its areas, the castle or the game complete.

Provenance: selected `dra/71830.c:{CreateCeilingHitEffect,PlayerStepHighJump}`,
`7879C.c` factory scheduling and `7E4BC.c` tile-particle behavior at pinned
sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
