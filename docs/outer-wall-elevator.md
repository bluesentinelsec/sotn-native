# Outer Wall elevator

The Wall shaft now runs the three-stop elevator, strike-operated activation lever,
rotating gears, remote door calls, collision tile changes and selected pressure
platform. Rides retain the original acceleration, reversal, stop correction and
arrival animations. Scripted input and player displacement use explicit native
services; the start of a ride stops both velocity axes. Stops and animations remain
in private stage tables. Existing version-six saves retain the two boolean mechanism
flags; decoding rejects invalid values without replacing the caller’s state.

The rope’s original off-screen draw-environment cache becomes a checked CPU texture
copy. Straight and swaying rope geometry, slanted opening/closing door panels and
selected elevator shadow use the existing texture pipeline. Quads now rasterize two
triangles with a top-left edge rule, retaining validation before framebuffer writes.
Persistent item/relic icons outside the viewport are culled before drawing; the
full-height descent check exposed the earlier far-off-camera icon bounds failure.

Gear ambience updates original channels 22/23 gain and stereo position without
restarting the sample loop or envelope. The original SDK register-volume overwrite
also applies when the channel starts. Public PCM tests compare gain updates against
uninterrupted playback, then silence/restore and pan the same loop.

## Validation and limits

Synthetic tests cover all stops, up/down travel, remote calls, door tiles/geometry,
weapon-driven lever activation, ownership/room cleanup, malformed variants and
saved flag validation. Renderer tests cover slanted quads and the rope cache. Save
regressions retain both flags in every supported save room and reject malformed
serialized flags. The existing public suite remains independent of game assets.

Private LLDB checks ride through all stops in both directions against original
room data, strike the lever with the ordinary selected weapon, and save/fresh-load
activation in an isolated directory. These are explicit protected fixtures, not
continuous-route or console parity evidence. Dummy audio checks decoding and lifetime,
not real-device listening. Complete sparks, switch debris, gear palette animation,
camera shake, per-primitive ordering and audiovisual/timing parity remain incomplete.

Adapted from NO1 `unk_38F50.c`, `unk_39184.c`, `unk_3B5E0.c`, DRA sound routines,
and SDK `SsUtSetVVol` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under AGPL-3.0-or-later.
All original tables, art, palettes, sound and game binaries remain private.
