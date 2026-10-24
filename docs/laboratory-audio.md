# Laboratory sound effects

The native Laboratory scene queues original sound IDs from attacks, player hits,
zombie behavior, candle breaks, and prize collection. A bounded C sound service
reads the original DRA event definitions and script records, resolves VAB programs
and tones, and advances 24 voices. It preserves sample ADPCM decoding, pitch,
Gaussian interpolation, and ADSR envelopes through reviewed voice code.
No sound samples, instrument definitions, or game scripts are compiled into the app.

The scene produces stereo 44.1 kHz PCM on simulation ticks and sends it to SDL3's
playback stream. It opens the user's default device, clears stale buffered audio
after debugger gaps, and pauses/clears playback when the scene loses focus.
Device initialization errors stop with the SDL error; they do not silently produce
a supposedly complete silent build. The simulated voices freeze while paused.

Additional private resources are required under the same game-data root:
`SD_J010.VB`, `VAB/SD_ALK.VH`, `VAB/SD_ALK.VB`,
`ST/NZ0/SD_ZKNZ0.VH`, and `ST/NZ0/SD_ZKNZ0.VB`.
They are loaded read-only and checked against public size/hash metadata.
Shared instrument headers remain in the private DRA image. Public source,
packages, and tests contain none of these inputs.

This is selected sound-effect integration, not full audio parity. Stage music,
SEQ playback, reverb, complete voice/channel arbitration, positional sound,
all movement/ability/ambient events, and original NTSC scheduling remain incomplete.
The direct/scripted paths exercised by the current combat checkpoint are supported;
other channel modes fail explicitly. Future hardware-service comparisons must
establish parity before removing these qualifications.

## Provenance and validation

`modules/audio/sound.c` adapts selected `src/dra/sound.c` event, script, and
channel paths at the pinned reconstruction revision under AGPL-3.0-or-later.
Pitch and volume follow the MIT-licensed PSY-Z `decomp/src/libsnd/vm_n2p.c`
and `vm_nowon.c` reference; the pitch curve is computed mathematically.
`voice.h` adapts selected PSY-Z `psyz/src/psyz/spu_voice.h` paths under MPL-2.0
at `b52003e55c0d1b20719e984dd19bd4d760eeb14a`. No SDK/platform headers were imported.
The accompanying Gaussian coefficients are public-domain mathematical reference
values, not game assets. Full notices ship in `licenses/` and `THIRD_PARTY.md`.

Public synthetic tests cover known ADPCM samples/predictor history, direct and
scripted playback, nonzero PCM, deterministic output, queue bounds, malformed
scripts, truncated bodies, and invalid VAB attributes. A private check rendered
11 original combat sound IDs to nonzero PCM, without publishing the recording.
A native candle break/collection check with SDL audio passed under LLDB.
These checks do not prove listening quality or complete audio/game parity.
