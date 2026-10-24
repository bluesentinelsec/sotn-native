# Laboratory audio

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

This is selected audio integration, not full audio parity. SEQ playback,
reverb, complete voice/channel arbitration, positional sound,
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

## Streaming Laboratory music

The scene now plays the original Laboratory track and loop from the user's
`SD/XA_STR1.` raw-sector container. Music selection, sector/filter IDs, volume,
intro duration, and loop duration come from the private DRA table. No music
recording or metadata array is compiled into the game. Room changes within
Laboratory retain playback instead of restarting the track.

`SotnStream` checks the entire container's size and revision fingerprint once,
using 32 KiB scratch memory, then reads bounded 2352-byte sectors. Startup logs
that verification is in progress; the large file is never loaded wholesale.
Extraction must preserve raw XA sectors, not just 2048-byte ISO data sectors.
`SotnMusic` validates matching sector headers, decodes the selected 4-bit stereo
37.8 kHz format, and sends it through an owned SDL rate converter. The scene
mixes the resulting music and effects before queueing playback. Pausing freezes
music simulation along with actors; unsupported formats and short reads fail clearly.

The SDL music rate converter and development tick clock are temporary services.
Original CD interpolation, seek/fade timing, reverb, and full audio parity remain
unproven. The music decoder adapts selected PSY-Z `psyz/src/psyz/libcd.c` paths
under MPL-2.0 at the same pinned backend revision. It replaces a negative signed
shift with defined multiplication and bounds predictor arithmetic and I/O.

Synthetic stream tests cover fingerprint chunk/padding boundaries, checksum and
size failures, rewinding, sector bounds, and unchanged output on failed reads.
Synthetic music tests exercise intro-to-loop and repeated-loop boundaries,
filter/format validation, and nonzero output through SDL conversion. A private
12,500-tick playback check covered the real introduction and two loop boundaries.
A native LLDB encounter with two zombie defeats and player HP decreasing to 60
passed with music and effects enabled. These are exercised-path checks, not
proof of complete listening quality or audiovisual parity.
