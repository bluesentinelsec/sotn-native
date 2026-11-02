# Familiar CD-command audio service

`SotnCdAudio` now owns selected original sound/CD command queues, acknowledgement,
XA start/expiry/stop, and the two saved music positions using actual native XA
decoders. It accepts10/11/A/90..94 and original US XA command identities301..532, with
actual private metadata/format validation before playback acknowledgement. The
eight selected US Fairy narrative IDs have actual [native producers](fairy-native-dispatch.md);
the wider audio service domain does not enable other controls. Ordinary Fairy
menu activation, saved progression and remaining general audio/cutscene lifecycle
remain unfinished. Ordinary grants and savev9 remain unchanged.

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
Its current intro/loop service remains the existing `SotnMusic` implementation.
Adoption retains that native service separately from newly queued managed music.
The command
owner applies the current absolute CD gain across native intro/loop metadata
changes and preserves the retained decoder on restore. It neither replaces
ordinary scene dialogue ownership by itself. Selected Fairy requests already bind the native app;
remaining general routing, reverb and full CD/session continuity remain open.

When the actual CD owner is already bound, native stage music, speech, stop and
pause/restore requests now share its command queue and decoder/bookmark ownership.
They no longer create overlapping legacy music/speech decoders. A stage restart
reads its real private profile before queuing that original identity. Pause precedes
a same-event speech request; a combined music/voice stop emits one original stop.
Stage dialogue waits bind actual XA activity and both acknowledgement flags.
Presentation reads use an immutable snapshot rather than replacing that feedback
with an absent legacy speech pointer after the stage callback.

Laboratory and Library boss managers now preserve their original90 fade requests;
shared dialogue opcodes9/20 preserve92 separately from stopA. The Laboratory manager
also retains the original state3-to4 increment when both bosses die during active
music; it reaches5 on the next callback. Other collapsed stage requests and global
stage-music flags still need their own source audits. Initial unbound bootstrap,
game-over audio, master-volume/reverb, whole sound-ring ordering and cross-session
CD continuity remain unfinished. This selected binding does not enable Fairy menus.

Selected native stage volume requests90..94 now reach this actual CD owner. Command8
preserves the immediate separate fade flag, signed-word counter, multipliers128/8/4/2/1,
original target-volume curve and deferred stop2. Changing the shared multiplier may
raise gain; one fade can clear the single flag while another remains queued. Neither
flag clearance nor queue exhaustion substitutes for the other.

Original `xa_play_ck` means pending fade or a playing identity, rather than decoder
readiness. Ordinary, spell-frozen and FFF stage actor snapshots and Fairy sound input
now bind that actual activity and separate busy flag. Remaining general stage music,
dialogue, master-volume fades80..84 and SDK reverb still require their actual owners.
A pending loop6 can outlive stop2's decoder retirement: it reconstructs its real
transport from retained validated configuration and reads at ReadN. Original loop6
does not enable serial mix; a playing identity may therefore coexist with serial off.

Newly queued mode1 music continues into actual adjacent private metadata without
seeking, restarting the resampler, discarding buffered PCM or clearing ADPCM history.
The original strict wrapping-clock expiry changes mode/duration and retains the
original playing identity. Mode0 expiry queues command6. Its actual new decoder is
configured at the private loop sector, reads real data at ReadN and only then supplies
PCM. Original serial-mix and busy/phase/deadline state remain distinct from native
data readiness. A pending12 inserts14 before6, including the original special
255-entry overflow path. New music below raw40 clears abandoned saved positions.
Mode/transport ownership survive real pause/restore bookmarks.

Managed decoding bounds its actual PCM to the private segment durations. Mode1
retains one cumulative intro/loop sample budget; a real seek starts the loop budget
in its new decoder. Exhaustion flushes the actual resampler tail and holds silence
until the original command owner seeks or stops. It never advances the playing state
early, substitutes another clock or scans unrelated tracks to fill the strict-deadline
gap. Corrupt matching-sector reads still fail explicitly. Existing adopted stage loops
and finite speech retain their separate transport behavior.

These managed loop services currently require an unchanged configured base. Global
queued-base retargeting fails before seeking; its actual filter/seek/restore ownership
remains unfinished. Remaining general native routing and SDK reverb are also
unfinished. Do not use these component services as ordinary familiar activation readiness.

Public authored checks cover nested saves/restores, third-save wrap, exact cursor
retention, finite tail/stop timing, wrapped clocks, both queue overflows, differing
intro/loop volumes and failed first-sector reads. Private original-instruction
comparisons cover full sound-ring/CD-queue contents, phases, acknowledgement,
playing identity, gain, deadlines and active bookmark metadata. Hardware CD,
VSync, serial-mix and volume services use explicit adapters. Native decoder
positions and existing stage loops remain separate evidence.
The earlier finite comparisons cover56 contexts,50,400 complete update sequences
and4,368 queued PlaySfx calls, including third-save wrap. The expanded private
comparison covers106 contexts,95,400 complete updates and5,532 queued calls,
including managed music, all five fades, shared-multiplier retargeting, retired-decoder
loop reconstruction and simultaneous loop/pause/254-255-entry overflow cases. It
also compares signed fade words, immediate flags and lost incoming-ring fade requests.
Mode0/1 expiry-clock fixtures isolate queue behavior; SDK reverb also uses an
explicit adapter. Those comparisons do not establish continuous playback by themselves.
Separate actual private-data cadence checks run three stage tracks for49,997 unskipped
updates, three metadata continuations, six real loop seeks and73,404,454 nonzero PCM
samples. They verify actual transport allocation/ReadN and retain the original playing
base. They are audio-service evidence, separate from native stage-actor routing,
menus, uninterrupted routes or console comparisons.

Actual native Debug and ASan checks route all five supplied stage volume events
through real decoding and command8. Seven menu updates hold player/game clocks while
CD fades advance. An actual five-MP Summon Spirit cast holds the game clock through
33 frozen updates and release while CD audio advances; ordinary MP recovery resumes
afterward. Repeated draws hold audio state. These card/load, protection, music-command
and stage-event fixtures establish selected native scheduling, not ordinary stage-script
routes, acquisition, full general audio, persistence or console parity.

Selected bound-stage routing checks pass767 paired native/reference-owner updates
in each Debug/ASan build, including separate and same-event pause/speech, natural
completion, retained stage resumes, real restart-profile selection and stop ownership.
No overlapping legacy decoder appears. The reference produces917,540 nonzero PCM
samples; this is service evidence, not device/console PCM equivalence. Its first new
start establishes a common deadline/mode after the old adopted transport's separate
prehistory. Original-instruction comparisons cover504 complete Laboratory/Library
manager calls, exact emitted audio identities and state transitions. XA activity and
the retained prior-phase music word are supplied fixtures; PlaySfx, TimeAttack and
debug printing use explicit callee adapters. Those checks are separate from actual
encounter routes or whole-pass acceptance.
A cached native candidate and real room17-to12-to16 entry also reach the placed
Laboratory manager, real Slogra/Gaibon roots and original90/CD8 retirement while
an actual menu holds player/game clocks. Supplied arrival/edge/floor positions are
not traversal evidence. The accelerated double-cross fixture reaches an unsupported
movement boundary on ordinary resume; uninterrupted room acceptance remains open.

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

The [native lifecycle owner](familiar-lifecycle.md) now supplies real event/entity/effect
ownership, owned loading, connected rooms and selected engine initialization.
Remaining integration must unify actual general stage/dialogue ownership, preserve
CD queues/bookmarks through their required session boundaries, own retargeted loop
contention and SDK effects, and finish whole-pass commit/rollback.
All six [Alucard completion gates](alucard-status.md) remain open.

Provenance: selected `dra/sound.c` PlaySfx, ExecSoundCommands,
AddCdSoundCommand, func_80133FCC, CdSoundCommand4/6/12, CdSoundStartXaPlayback and
CdFadeOut2 at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`
(AGPL-3.0-or-later). Existing XA decoding retains its separate PSY-Z provenance.
All metadata, speech and music remain private.
