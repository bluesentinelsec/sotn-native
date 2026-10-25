# Chapel Maria encounter

Royal Chapel's internal checkpoint now runs the placed original Maria dialogue,
its event-created scene controller and Maria actor. The scene positions normal
Alucard, pans to frame both characters, plays five original speech clips, presents
private dialogue/portraits, runs Maria's two departure jumps and returns player
control and the camera. Its original completed-encounter flag survives ordinary
version-eight saves; revisiting the room after Continue does not replay it.

This advances the internal Chapel checkpoint. Hippogryph, connected Chapel travel,
remaining castle progression, transformation approaches and complete audiovisual/
timing parity remain incomplete. It is not a completed area or game milestone.

## Original state and ownership

The dialogue uses its own DAI script, event stream and portrait palette records.
Raw actors 0x45, 0x46 and 0x47 retain their original identities. The script's event
factory positions actors in room coordinates: the source subtracts tilemap scroll
for screen positions, while the native room coordinate system already includes
that scroll. Do not add the camera a second time. Existing Laboratory, Entrance
and Library dialogue profiles retain their separate offsets, identities and flags.

The controller holds simulated left input until Alucard reaches X 368, then aligns
his whole-pixel position while retaining its fraction and movement state. This is
an explicit bounded script alignment, separate from small environmental shoves.
The camera's original horizontal anchor advances one pixel per simulation update
toward 176 and later returns to 128. Camera bounds and vertical follow remain
active, and room entry restores the default anchor. Presentation never advances
these clocks.

Original pause/map and stopwatch gates remain distinct from generic actor freezes.
The controller restores both gates when its pan and departure flags finish. Input
resumes after its final simulated-pad tick. Transformation handling is deferred
with the player's other unintegrated transformation abilities. The original
seen-cutscene settings and conditional replay-skip behavior remain incomplete;
the first-time scene cannot be skipped through a newly invented unconditional key.

Maria reads her original private step, run and jump animation streams. Her run
uses 1.5 pixels per tick and her jumps retain original vertical speed and gravity,
landing heights and prior-position comparisons. Original footstep poses request
volume 80 and bounded stereo pan. Animations, scripts, text, artwork, sound banks
and speech bytes remain private; public tests use synthetic data.

## Presentation and persistence

Chapel portraits upload to their original page 0x90. Library and Laboratory keep
their separate pages. Both slots and mirroring use the existing bounded native
texture service, including validation before upload. Speech IDs 0x375 through
0x379 select finite original XA clips through verified private DRA metadata.
The existing native speech and music owners handle pause/resume independently;
reverb, exact CD scheduling and full dialogue-box/name/typography parity remain
unproven.

Completion sets original castle flag 99, not the Entrance or Laboratory encounter
flags. Version-eight saves already retain that castle flag array, so no format or
application version change is needed. Invalid values for the newly integrated
boolean flag fail validation without replacing an existing save or decoded state.
Transient actors, camera pan and dialogue cursors are not ordinary save state.

## Validation

Public synthetic cases cover script/event coordinates, portrait palette/page,
voice waits, distinct completion flags, replay suppression, original motion and
footstep requests, camera pan/reset/bounds, script alignment/fraction retention,
pause restoration and invalid state/data. Renderer cases check both Chapel
portrait slots and mirrored pixels; speech cases exercise finite completion.
Save cases retain the encounter and unrelated flags and reject corrupt values.
Existing dialogue and camera regressions remain required.

A private native LLDB sequence starts explicitly at the room's right entry with
the ordinary development character. It runs the placed script without forcing
actor poses, observes both camera anchors, five original clips, dialogue/portraits,
Maria's run and two jumps, input gating and return of normal control/music. Each
original speech clip also decodes nonzero PCM separately. Private rendered captures
are inspected and kept out of public source and artifacts.

The sequence then uses a controlled room rebase to a Chapel save room, completes
its actual slot/confirmation/coffin write through an isolated store, runs fresh
Continue, and revisits the event room to check suppression. Existing user save
slots remain untouched. Explicit starts and room rebases are fixtures, not an
uninterrupted castle route, listening comparison or proof of console parity.

All 95 public groups pass in Debug, macOS ASan/UBSan and Release; packaging
passes. Debug and sanitizer native runs each pass 2,076 simulation ticks and
519 regular rendered frames, including isolated save/fresh Continue. Shared
private regressions pass the earlier Laboratory Maria scene and its save reload,
the Librarian introduction, three Wall/Library connectors with isolated save
retention, and all four confessional branches with chime restoration.
