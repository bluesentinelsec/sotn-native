# Familiar CD-command audio service

`SotnCdAudio` now owns selected original sound/CD command queues, acknowledgement,
XA start/expiry/stop, and the two saved music positions using actual native XA
decoders. It accepts10/11/A and the eight US Fairy narrative IDs. Native Fairy
slots, event production, selection and rescue remain unavailable; this service
does not enable controls. Ordinary grants and savev9 remain unchanged.

The256-entry sound-command ring and separate256-entry CD queue preserve original
read/write order, clearing, duplicate-clip early return and distinct overflow
behavior. PlaySfx10/11 sets the acknowledgement-busy flag immediately, before the
command reaches the CD queue. Queue-empty, acknowledgement and playing-XA are
separate snapshots; decoded PCM completion is not a replacement for any of them.

Pause retains the original volume-minus12 and command phases, saving the actual
decoder with its stream position, ADPCM history and pending SDL converter samples.
Restore uses that decoder, original volume-plus12 and later acknowledgement.
Two positions permit nested interruption. A third save wraps the original cursor
and releases software decoders for the abandoned positions. It does not invent a
larger stack or reserve another voice pool.

Native configuration/filter/position validation and actual first-sector reads
back the selected CD operations. A corrupt matching sector fails ReadN before
playing/acknowledgement completion. Valid synchronous native operations advance
the corresponding original phases; no guessed CD-ready timer is used. Original
hardware seek latency, CD-location bytes, interpolation and raster/audio timing
remain outside this service's evidence. Retaining the SDL decoder is the native
position service, rather than an emulated CD-drive location or reconstructed
hardware resampling history.

Finite speech retains private mode2's twenty-tick tail, the original strict
elapsed-greater-than-duration test and subsequent volume-minus32/pause/retirement
phases. It uses the caller's wrapping engine/VSync clock. The decoder can finish
its PCM while original XA-playing remains set; the owner holds silence through
the remaining phases. Actual audio scheduling through menus/freezes and session
handoff still belongs to the native engine integration.

Existing native stage music can be adopted without restarting its decoder.
Its current intro/loop service remains the existing `SotnMusic` implementation;
this batch does not reconstruct original CD mode0/1 loop commands. The command
owner applies the current absolute CD gain across native intro/loop metadata
changes and preserves the retained decoder on restore. It neither replaces
ordinary scene dialogue ownership nor binds Fairy requests to the native app yet.

Public authored checks cover nested saves/restores, third-save wrap, exact cursor
retention, finite tail/stop timing, wrapped clocks, both queue overflows, differing
intro/loop volumes and failed first-sector reads. Private original-instruction
comparisons cover full sound-ring/CD-queue contents, phases, acknowledgement,
playing identity, gain, deadlines and active bookmark metadata. Hardware CD,
VSync, serial-mix and volume services use explicit adapters. Native decoder
positions and existing stage loops remain separate evidence.
The final comparisons cover56 contexts,50,400 complete update sequences and4,368
queued PlaySfx calls, including third-save wrap. Debug and ASan traces match.

Native backend checks also connect the actual service snapshots to the original
Fairy DE sound-state component, including natural completion, cancellation,
cutscene retirement and special-room omission of restore. Reference PCM/cursors
verify retained stage decoding across interruption. These supplied event/room/
cutscene contexts do not establish actual slots4..7, a native Fairy producer,
ordinary acquisition, uninterrupted routes, rescue or console parity.
Both builds pass32 contexts and3,796 joint callbacks, with344 reference-identical
stage PCM/cursor updates,16 retained-decoder resumes and4,062,441 nonzero samples.
All220 authored tests pass in Debug, Release and ASan/UBSan;34 Python tests,
repository policy, formatting, static SDL linkage and Release ZIP checks pass.
Both actual CMake-built apps also pass the existing owned-medicine regression.

The remaining integration must bind the actual event lifecycle and engine audio
clock, commit requests after successful owner updates, retain or replace real
music ownership across sessions, and draw/retire actual shared Fairy effects.
All six [Alucard completion gates](alucard-status.md) remain open.

Provenance: selected `dra/sound.c` PlaySfx, ExecSoundCommands,
AddCdSoundCommand, func_80133FCC, CdSoundCommand4/12, CdSoundStartXaPlayback and
CdFadeOut2 at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`
(AGPL-3.0-or-later). Existing XA decoding retains its separate PSY-Z provenance.
All metadata, speech and music remain private.
