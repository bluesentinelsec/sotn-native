# Librarian sequence audio

The sound service now decodes the original Library SEQ track and plays its notes
through the existing native SPU sample/envelope service. The native scene attaches
verified private sequence input and exposes independent sequence start/stop events.
The librarian/shop actors do not issue these events yet; Library travel remains
gated while that interaction and other scripts are integrated.

`ST/LIB/SEQ_LIB.SEQ` is an additional on-demand private resource. Its filename,
length and fingerprint live in the reference manifest; its event bytes never enter
source, tests or packages. Users supply this file from their own matching dump.
It borrows the existing common sound bank, not the stage-enemy bank. Private DRA
settings select its original bank, volume and repeat mode. SDK pitch entries are
read from the verified boot image; no musical data or pitch table is embedded.

The decoder retains original 60-Hz integer scheduling, distinct initial/meta tempo
rounding, variable-length deltas, running status, program changes, zero-velocity
note release, pan/volume, pitch bend and selected NRPN/Data Entry loops. PS1 meta
events do not carry standard MIDI length bytes. This is a selected SEQ service,
not a general MIDI player. Unsupported commands and malformed input fail clearly.
An instruction budget rejects immediate infinite loops that would hang an update.

Sequence voices share the original selected sixteen-voice allocation range with
fixed effects. Tone note ranges, priority/envelope/age selection, original pitch
rounding, tone/program/channel/sequence gain and stereo pan remain explicit.
A direct effect key retires the previous sequence binding on that hardware voice;
stopping a sequence releases only its current notes. The retirement service retains the SDK's original fifteen-bin intersection in a
sixteen-bin envelope history, including its skipped final bin. Release clears the
allocation claim while retaining the identity for subsequent channel updates.
Envelope sampling occurs once per service tick; exact hardware flush/steal timing,
reverb and whole-hardware PCM parity remain unproven.

Sequence events remain independent of XA music and speech. Starting/stopping a
sequence cannot implicitly stop XA, and stopping XA cannot implicitly stop the
sequence. This matches the separate original sound command paths. The current
scene has no new public start mode, save format, game enhancement or version bump.

Public synthetic cases cover timing, running status, finite/infinite loop controls,
tempo changes, note release, exact selected gain/pan values, pitch, actor-independent
voice ownership, pool exhaustion/priority, invalid input and callback failure.
All 71 public groups pass in Debug and macOS ASan/UBSan; packaging passes.

A private comparison runs 100,000 original SDK scheduling ticks and matches 74,336
note/bend events, including cursor/delta/tempo state, internal loops and tempo changes.
The selected Data Entry branch is checked against the original private executable;
other unsupported branches are excluded explicitly. A further 50,000 note/fine/
center/shift cases match the SDK pitch algorithm using the original table. These
are algorithm comparisons, not console audio capture comparisons.

A private LLDB sequence opens the native Library scene, starts the original sequence
and independently stops XA, renders 3,600 audio service ticks with one internal
rewind, then stops the sequence. It checks nonzero PCM and sixty scene renders.
Debug and ASan/UBSan produce identical PCM hashes and exit successfully. No recording
is published. This checks the sound/scene boundary, not librarian interaction, listening
quality, an uninterrupted route or full audiovisual parity.

Algorithms adapt MIT-licensed reconstructed `main/psxsdk/libsnd/seqinit.c`,
`seqread.c` and selected `vmanager.c` functions at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. Original playback selection follows
`dra/sound.c` (AGPL-3.0-or-later). Existing SPU decoding remains under its documented
PSY-Z provenance; read [the sound service](laboratory-audio.md).
