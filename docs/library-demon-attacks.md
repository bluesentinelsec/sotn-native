# Selected Lesser Demon attack services

The Library adapter now has Lesser Demon's selected spit and bolt children. The
[selected boss body](library-lesser-demon.md) now commands them with its summoning
and [minion services](library-minions.md). Placed entry and reward remain pending.
These children also retain explicit component fixtures; they do not make the placed boss
encounter or Library route available. Stage 2 remains gated.

Spit retains its initial direction delay, diagonal flight, private floor sensor,
impact animation, expanding damage column, shrinking UV/geometry, sound and fade.
Its private enemy definition supplies damage and elements. [Selected poison mechanics](player-poison.md) now have separate seed-contact checks;
complete damage/effect parity remains pending. This attack check is not poison validation.
Optional primitive exhaustion retires the effect as the original does.

The bolt retains its native sprite, sixty-record budget, colored textured head,
three initial trails, fractional line endpoints, quantized SDK direction changes,
link spawning and independently fading color channels. Exhausted link allocation
preserves the original byte timer's wrap/retry. Released packet slots can be reused
later in the same actor update. Ownership is native and generation checked.

The US reconstruction reads an uninitialized packet pointer when assigning the
head's initial fourth vertex. Native C initializes that vertex to the first
vertex's X, matching the PSP path and every later US update. This is an explicit
memory-safety adaptation; first-frame console visual parity remains unproven.
No original game bytes, lookup arrays, animation streams or artwork are compiled in.

The native stage renderer also accepts untextured flat/Gouraud quads for the later
summoning effect. They use direct RGB555 color interpolation and original blend
modes; black is a drawable color. Texture transparency rules still apply only to
textured packets. Bounded geometry validation precedes writes. Full PS1 raster,
dithering and blending parity remain outside this selected path.

Public synthetic cases cover both spit directions, impact/column hitbox timing,
UV changes, fade, both bolt directions, branching, slot reuse, pool exhaustion,
byte timer wrap, malformed animation/math and fixed-point overflow. Renderer cases
cover gradients, black, all four blend modes, shared triangle edges and failed
writes. All 68 groups pass in Debug and macOS ASan/UBSan.

Private LLDB fixtures insert both children in Library room 0, whose original room
bank contains the boss artwork. They draw flight, column, trail and retirement,
plus a synthetic colored quad through the native stage adapter. Debug and macOS
ASan/UBSan agree at 180 renders and exit successfully; release packaging passes. The protected
character and inserted children are component evidence, not ordinary boss inputs,
full fight/route, poison behavior or audiovisual/console parity.

Algorithms adapt pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`,
`st/lib/e_lesser_demon.c:{EntityLesserDemonSpit,func_us_801BCC10,func_us_801BC814}`
and selected shared stage helpers (AGPL-3.0-or-later). All private tables and assets
remain the user's runtime inputs.
