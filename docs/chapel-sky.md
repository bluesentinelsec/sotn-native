# Chapel cloud and sky backgrounds

Royal Chapel now presents its selected original scrolling cloud planes and blue
sky-land strip. The internal Chapel profile remains disconnected while its
remaining scenery, scripts and boss need integration. This is not a complete
stage or authentic-game milestone.

The actor adapter follows `st/e_clouds.h` and `st/e_background_sky_land.h` at the
pinned reconstruction. Its 96 cloud primitives retain three independent wrapped
16.16 scrolling phases, unsigned depth/row phase, signed horizontal phase,
eight perspective bands per plane, original colored near/far vertices and
ordering priorities. The selected SDK divider/saturation adapter projects the
private plane vertices with the original identity rotation, screen distance and
screen offset. No host floating-point projection replaces that path.

Cloud UV records, models, grid pointers, grid cells and plane height/priority
records remain in the user's private DAI or BO5 overlay. Each stage selects explicit
metadata locations; BO5 does not borrow DAI's data or masquerade as stage six.
The native reader validates
their sizes, address domains, alignment and values before use. Synthetic public
tests deliberately vary those records. The existing scene backdrop service
applies the original cloud clear color; screen-space cloud primitives keep the
actor's room position distinct from camera-adjusted projection coordinates.
Room entry resets the backdrop before new actors initialize, following
`dra/5087C.c:func_800F2404`; the previous room's cloud color cannot leak indoors.

The scan retains leftward search, reversal, sparse-cell skipping and rightward
termination, with the original primitive cap and hidden unused tail. An all-zero
row cannot terminate the original loop. The native reader rejects it with an
explicit error and bounds the scan rather than hanging on malformed inputs.
This is an explicit memory/termination adaptation, not a change to the verified
original grids. Phase arithmetic uses defined unsigned wrapping rather than host
signed overflow.

The sky-land actor owns five screen-space textured strips, preserving its
original UVs, palette, priority, fixed height and repeated widths. Both actors
omit their optional presentation when the primitive pool is full, as the source
does. Room/generation ownership clears their reservations. These transient phases
do not change v8 ordinary saves or grant any player progression.

Chapel/BO5 sky packets now precede both room tile layers. Their selected priorities
(land 16, clouds 24/28) are below the original tile buckets (background starts at 32,
foreground at 96 in the checked rooms). This corrects earlier presentation that let
clouds cover opaque architecture. Library's separate parallax pass remains before
its foreground layer.

## Validation and remaining work

Public synthetic tests cover alternate private models/profiles/UVs, perspective
and screen saturation, wrapping phases, camera-adjusted coordinates, dense and
sparse scans, hidden-tail cleanup, land geometry, pool exhaustion and malformed
pointer/index/model/rotation/state boundaries.
Room-entry tests also clear the prior backdrop and transient cloud owner/phases.
All 92 groups passed in Debug, macOS ASan/UBSan and Release; packaging passed.

A private native LLDB check starts the original skyway room with a protected
development character. Its placed cloud/land actors run 120 ticks and 30 rendered
frames, retaining exact per-tick phases during camera movement. A diagnostic
temporarily hides cloud primitives and then restores them; 23,549 screen pixels
differ, establishing that their actual texture/palette/render path contributes
to the frame. The full-art capture was visually inspected. This protected,
explicit-start component fixture does not establish ordinary traversal,
uninterrupted-route or console visual/timing parity. Dummy audio exercises
ownership and decoding, not listening or PCM parity.
Debug and sanitizer native sky checks passed. Shared native regressions also
passed four tower crossings and three Wall/Library connectors, isolated ordinary
save/Continue and missing-destination owner retention. Real user save slots remain
untouched.

An additional private native LLDB sequence runs 120 ticks and 30 renders each in
Chapel's skyway and BO5's arena. Hiding/restoring the 101 original cloud/land
primitives changes visible background pixels while preserving every opaque tile
pixel in both checked rooms. Public tests vary BO5's model independently of DAI,
reject cross-stage grid references, and retain projection/state boundaries. This
does not establish complete ordering-table or console image parity.
All 99 current test groups and these two native scenes pass in Debug and macOS
ASan/UBSan. The earlier pixel count above describes the previous Chapel-only check;
the corrected tile ordering intentionally changes that image.

BO5 dialogue, connected Chapel progression and full GPU/effects/audio/timing parity
remain incomplete. Selected stained glass, confessional and Chapel Maria services
are documented separately in [Chapel runtime](chapel-runtime.md).
