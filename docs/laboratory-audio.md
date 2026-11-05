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
Private data verification and decoder initialization finish before the device
opens. Playback starts only after the first PCM tick is queued; focus changes
and debugger gaps rearm this same behavior instead of resuming an empty stream.
Device initialization errors stop with the SDL error; they do not silently produce
a supposedly complete silent build. The simulated voices freeze while paused.

On macOS, a guarded adjustment to pinned SDL avoids a redundant asynchronous
playback stop before queue disposal. An SDL-only playback/close reproduction and
native LLDB launches previously logged `AudioQueueInternalStop_Sync -66671`;
disposal without the pending stop removed that warning across 20 device lifecycles
and local game launches. A paced native session also exercised focus loss, resume,
a debugger-sized gap, and shutdown under LLDB.
Startup CoreAudio scheduling warnings can still occur under LLDB. These checks
do not establish that every OS diagnostic is harmless or prove audio parity.
Synthetic device tests cover paused creation, invalid PCM, focus/debugger reset,
repeated playback/close, initialization failure, and SDL subsystem ownership.
CI also checks patch application, repeatability, and rejection of source drift.

Additional private resources are required under the same game-data root:
`SD_J010.VB`, `VAB/SD_ALK.VH`, `VAB/SD_ALK.VB`,
`ST/NZ0/SD_ZKNZ0.VH`, and `ST/NZ0/SD_ZKNZ0.VB`.
They are loaded read-only and checked against public size/hash metadata.
Shared instrument headers remain in the private DRA image. Public source,
packages, and tests contain none of these inputs.

Explicit original volume/pan now carries through direct and scripted events, using
private DRA coefficients. SDK tone/program/event panning attenuates the correct channel
without replacing the other channel's gain. Selected clock sounds exercise this path.

This is selected audio integration, not full audio parity. The selected
[Librarian SEQ service](library-sequence-audio.md) now plays notes and internal loops.
Other sequences, reverb, complete voice/channel arbitration, complete positional sound,
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
scripted playback, nonzero PCM, deterministic output, invalid queue inputs, malformed
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

The [selected spell script-pause service](spell-sound-freeze.md) now retains original
queued pause/resume, non-pausing scripts and copied cursor/allocation behavior.
The native spirit-casting owner and full audio parity remain incomplete.


## Selected effect ring

The actual sound owner now retains the original256-slot read/write ring across
updates. PlaySfx/PlaySfxVolPan advance the write cursor without a capacity error;
256 unconsumed writes make the cursors equal and leave no pending effects.
A subsequent write exposes only its new retained span. ExecSfx clears each current
ID, scans the remaining span and dispatches only the last matching ID, retaining
that request's relative order, volume and pan. Consumed volume/pan words remain
in their actual slots. MuteSound initialization clears both cursors and records;
prepared familiar SPU replacement copies the live ring without an extra update.

The existing array supplies this ownership; no second effect/voice pool is added.
Bound native SPU controls still use the actual separate
[CD sound-command ring](familiar-cd-audio.md). Standalone pause/resume/release and
queued ambience gain retain their selected compatibility API in the sound queue.
Their shared fallback storage is not evidence of full original global command
ordering or ambience-command ownership.

Public authored tests cover last-request script order and gain/pan, lookahead
across the physical end,256-wrap silence,257th-write selection and invalid-input
holds. Private Debug/ASan production-controller traces match8 contexts,
20,648 requests,96 drains and2,751 surviving dispatches against complete original
PlaySfx/PlaySfxVolPan/ExecSfx/IncrementRingBufferPos instructions. All256 ID/volume/pan
records and both cursors match after every call; Debug/ASan traces are identical.
The effect-dispatch callee is an explicit adapter in this ring comparison; scripts,
voices, DSP and source routes are separate evidence.

Actual native Debug/ASan checks compare43 command/render pairs against independently
queued surviving requests using real private scripts and voice/DSP/RAM/PCM owners.
They cover last-request order and gain/pan,256-wrap silence,257th-write selection,
eight menu updates, an ordinary five-MP Summon Spirit cast with33 frozen updates
plus release, and five repeated draws retaining complete sound state. Arrival,
protection and supplied effect bursts do not establish ordinary effect producers,
acquisition, uninterrupted routes, hardware DSP or console parity.

All eight final CD/rescue/game-over/actual-app regressions pass in Debug/ASan
with normal exits and no sanitizer errors. All220 public tests pass in Debug,
Release and macOS ASan/UBSan alongside Python, policy and package checks.

Full direct-effect voice allocation and retained identities, key status/SDK key
timing, universal bootstrap, CD/SEQ/dialogue/session continuity, other presets,
remaining positional/movement/ambient producers and NTSC scheduling remain open.
This batch does not close any of the six [Alucard milestone gates](alucard-status.md).
Provenance remains the pinned AGPL source noted above; all private inputs and traces
remain outside public source and packages.


## Selected direct-voice allocation and status

The existing24-voice owner now retains the original four direct-effect identities
and priorities for12..19. Outside SEQ access, allocation checks the same ID,
then a free identity, then strictly lower priorities across all four groups,
then equal priorities across groups0..2 only. The equal pass intentionally
excludes group3. With retained open SEQ access, direct effects use only18/19:
a free identity wins, otherwise the request requires equal-or-higher priority.
A decoder-active flag no longer invents a free allocation.

Actual rendering now mirrors the computed envelope into each voice's ENV register
word. Before effect dispatch, native audio samples these words and reclaims the
selected direct identities and ended script leases in original order. It retains
ordinary groups while SEQ access selects group3 and preserves the distinct fourth
script key-off12..15 versus playback14..17 ranges. Rendering itself does not retire
allocation metadata. Script playback uses that held status snapshot, checks only
each pair's first/even voice and keeps one shared KeyOnRange fallback cursor. A
busy range can use a cursor retained from another range. Free selection advances
or wraps the cursor; fallback retains it. Both cursor and status cache survive
all-voice release and full mute, without another sample. The selected SDK software
key mask starts at zero and is only
cleared by this owner; SDK note-on flushes write hardware registers directly.
Consequently a nonzero actual ENV word reports selected status2, and zero reports0.
Other SDK software key-on producers and full SDK/NTSC flush timing remain unowned.

Channels20/21 and22/23 now retain their own current IDs, gain and signed distance.
Release mode2 applies real22/23 RR8 writes and clears that identity. Bound and
standalone A1 share actual20/21 RR14/key-off and identity clear, retaining gain/pan. All-voice
InitSoundVars2 clears current identities/priorities while preserving gain/distance,
copied scripts and pending effects; full MuteSound/InitSoundVars1 also clears the
gain/distance words. No second voice pool or new effect availability is introduced.

Authored tests cover repeated identities, strict-before-equal selection, the
ordinary group3 equal-pass exclusion, open-SEQ restriction, real ENV-word sampling
before allocation, ambience metadata and release-versus-mute resets. Existing
lease fixtures now supply the actual status words rather than decoder flags.

Private Debug/ASan traces are identical across all736 private definitions and
8,832 seeded contexts:17,664 allocation calls,17,664 status-reclamation calls,
8,832 A1 releases,8,832 all-voice releases,4,416 full resets and14,112 pair requests match original
instructions. Complete original allocation/KeyOn12_19/script constructors,
reclamation and reset functions run; actual SDK key-status and RR functions execute
against supplied ENV words and MMIO. Tone-pair playback and gain-only SDK callees
are explicit adapters. Seeded identities/cursors/status/register vectors remain
component evidence, separate from native playback or console fidelity. A separate
512-case Debug/ASan comparison executes the complete virtual30..33 dispatcher and
KeyOnRange: all range-status patterns, all eight retained global cursors, even-only
free selection, wrap, cross-range fallback and immutable status cache match.
Its two SDK tone-on calls are explicit adapters; traces are identical.

Actual Debug/ASan native checks separately pass1,806 reclamations,24 allocations
and one bound A1 release, including18 direct and six script/release calls, with
byte-identical traces.
Original whole calls match those real native states and rendered ENV snapshots.
Reclamation and script/release compare145 state fields; direct births compare97
metadata fields before native tone/DSP changes. The pair/gain adapters do not stand
in for completed audio rendering.

The native session also queues12 real stereo-effect requests from six verified
loaded pairs, exercises closed and actual private Library-SEQ pressure, preserves
the other three identities during open access and closes through queued7. A
separate real20/21 pair verifies bound A1 clears its ID, preserves gain64/pan-3
and applies RR14.
Its1,075 measured final-mix frames retain five master fades, two measured SEQ closes,
menu8, ordinary five-MP Summon Spirit with33 frozen updates plus release, and
25 repeated-draw holds. Source/allocation matches and actual native PCM/mixer checks
retain their different scopes. Arrival/card/load/protection/audio/tone fixtures
do not establish ordinary effect acquisition/producers, traversal, full SDK flush
timing or console parity.

All220 tests pass in Debug, Release and macOS ASan/UBSan alongside Python, policy,
formatting, static linkage and Release ZIP CRC/private-input checks. All eight final
Debug/ASan CD, rescue, game-over and actual medicine-app regressions pass with normal
exits and no sanitizer errors. Exact-head platform results accompany the PR.
Full SDK voice pressure/key timing, remaining global sound controls, CD wet sends,
cross-session CD/SEQ/SPU continuity and all six Alucard milestone gates remain open.


## Selected inventory-menu ambience controls

The bound CD/SPU sound ring now executes original commands1 andA0..A8 on the
existing24 voices. A0 releases all24 with RR14 and applies InitSoundVars2.
A1/A5 release20/21 or22/23 with RR14 and clear that pair's current ID; A2/A6
use RR8 and clear it. A3/A7 use RR8 only when an ID exists and retain it.
A4/A8 rekey the retained private definition with its current gain and distance;
22/23 also receive the original gain-only register writes. These operations
retain their original script-copy, parameter and no-sample-tick boundaries.

Actual inventory CLOSED/open transitions now queueA7,A3,E on entry andF,A4,A8
on exit. Both Start and main-menu Cancel close paths reach that owner. Audio
continues while the player/game clocks hold; repeated draws advance neither.
Map and spirit-freeze transitions retain their own source boundaries.

Native stage ambience changes now update retained22/23 gain/distance immediately
and queue command1 in the same sound ring. No current ID means no update or queue
write. Execution uses the then-current ID, including private entry0 if an earlier
release cleared it. Later parameter changes therefore affect earlier pending1
commands, and duplicate-XA returns can hold them. Standalone sound API compatibility
keeps its separately documented deferred/no-active-ID behavior.

Public authored checks exercise absent IDs, retained gains, release/rekey, all24
cleanup and queued gain after ID clearing. Private comparisons execute the complete
original command dispatcher and SetVolumeCommand22_23, with real SDK RR writes;
pair playback and gain SDK calls remain explicit adapters. Actual native checks
use loaded private20/22 pairs, menu input, combat gain requests and PCM separately
from those component fixtures. The private command comparison passes13,260 contexts,535,500 queued commands,
27,540 dispatcher calls and13,260 immediate-gain requests, comparing405 state
fields. All seven private20/22 definitions,17 distances, three gains, empty IDs,
wrapped cursors,256/257-write overflow and duplicate-XA holds are covered.
Debug/ASan traces match byte-for-byte. Actual native Debug/ASan checks pass two
menu cycles with both loaded pairs,16 audio updates with held player/game clocks,
six draw holds and a real stage-combat gain request. Fourteen actual native command
snapshots match the original dispatcher, with gain requests checked separately
from adapted pair playback. These checks preserve the fixture boundaries above.
Full SDK flush/NTSC timing, CD wet sends, remaining controls, session continuity
and milestone1 acceptance remain incomplete. No new
pool, default grants, savev9 fields or ordinary Fairy menu activation is added.
