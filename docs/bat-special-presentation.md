# Bat special-ability presentation components

The native core now reconstructs the selected Wing Smash and Fire BlinkWhite
flashes and the shrinking Wing Smash trail. These components do not yet enable
special abilities in gameplay. Actual shared factory/attack integration follows
the [command/motion component](bat-command-motion.md); ordinary Bat flight keeps
its existing runtime. No relics are granted to ordinary starts.

Fire uses original strip packets and profile five. Wing Smash uses the original
`0x5C` fan with profile `0x1C`: eight boundary edges, moving center, per-edge colors,
transparent center and private SDK rotation. Mirrored fan geometry retains its
own edge rule, distinct from the one-pixel correction for ordinary strips. Shared
phase angles advance in original child order. A new transformation lock retires
these old flashes before they can conflict with its capture geometry.

The `0x7009` hold deliberately falls through the original `0x700B` condition.
It can increment the phase twice when the supplied owner leaves Wing Smash,
reaching phase four. The original switch performs no phase-four action; geometry
continues until a later real removal condition. Preserve this behavior rather
than adding an invented fade or completion timer. Owner step/substep and raw
status remain separate inputs.

Trail children retain their actual factory position while following Alucard's
current frame. Birth captures facing, rotation, draw flags and priority; later
owner changes never rewrite those fields. Original scale drops by eight and
opacity by five per tick, retaining the final under-five removal rule. Leaving
Wing Smash removes the child even if Alucard now uses normal artwork. Geometry
scales signed-word distances before SDK rotation, preserves negative rounding,
and samples the real player display slot with the original blue-outline palette
and additive modulation. It uploads no substitute or frozen texture.

All 134 public tests pass in Debug and macOS ASan/UBSan, and the Release
packaging suite passes. Two duplicate standalone Release checks collided with
packaging on shared authored fixture files; both pass when rerun sequentially.
Authored fixtures cover fan edges/center, color interpolation, normal and retained
phase-four lifetimes, transformation locks, Fire removal, trail motion/retirement,
captured versus live attributes, negative scaling, mirror UVs and atomic failures.
Private muted LLDB comparisons cover 192 supplied normal/Bat image contexts and
14,080 flash controller/shared/quad records, including the earlier transformation
regression cases. Another 96 contexts cover 1,920 trail controller and actual
private-art SDK scale/rotation packets. Original controller, private artwork,
geometry, profiles and math execute; allocation/destruction and SDK leading-zero
instructions use explicit adapters. These are component checks, not live ability,
shared-pool, raster or console parity. Debug/sanitizer traces are byte-identical;
both actual CMake app builds pass ordinary Bat regression checks through muted
LLDB and exit normally. No user save slots are written. Runtime inputs/traces stay private.

Provenance: selected `dra/7879C.c:EntityPlayerBlinkWhite/EntityWingSmashTrail` and
`4B758.c:func_800EB758/RenderEntities`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
