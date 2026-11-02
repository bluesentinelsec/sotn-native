# Familiar CD-command audio service

`SotnCdAudio` now owns selected original sound/CD command queues, acknowledgement,
XA start/expiry/stop, and the two saved music positions using actual native XA
decoders. It accepts10/11/A/80..84/90..94 and original US XA command identities301..532, with
actual private metadata/format validation before playback acknowledgement. A bound
SPU also supplies selected7/E/F/A1/202/204 controls through this same ring. The
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
remaining general routing, CD reverb and full CD/session continuity remain open.

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
game-over audio, CD reverb, whole sound-ring ordering and cross-session
CD continuity remain unfinished. This selected binding does not enable Fairy menus.

Selected native stage volume requests90..94 now reach this actual CD owner. Command8
preserves the immediate separate fade flag, signed-word counter, multipliers128/8/4/2/1,
original target-volume curve and deferred stop2. Changing the shared multiplier may
raise gain; one fade can clear the single flag while another remains queued. Neither
flag clearance nor queue exhaustion substitutes for the other.

Original `xa_play_ck` means pending fade or a playing identity, rather than decoder
readiness. Ordinary, spell-frozen and FFF stage actor snapshots and Fairy sound input
now bind that actual activity and separate busy flag. Remaining general stage music,
dialogue and CD reverb still require their remaining owners. Bound master fades80..84
now use the actual owners described below.
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
remains unfinished. Remaining general native routing and CD reverb are also
unfinished. Do not use these component services as ordinary familiar activation readiness.

The selected [native SPU voice-reverb owner](spu-reverb.md) now supplies actual SDK
Studio B processing and selected SEQ depths. Bound mode0 expiry now commits actual
temporary depth127 before queueing6; command6 restores the current original depth
byte before playback completion. The restore byte starts at BSS zero independently
of the current SDK gain. Prepared SPU replacement rebinds the actual dependency;
failed binding preserves the previous owner and advances no clock.

Actual native sound command updates precede CD commands and voice/DSP rendering,
so these depth changes reach the same audio frame. Rendering retains the script
lease span captured before SEQ updates, including an ending sequence's fourth
channel. CD sends, global sound-ring ordering and remaining mixer
integration still need their actual owners.

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


Bound native master requests80..84 now enter actual command10 with shared32/8/4/2/1
multipliers, the immediate single fade flag and signed master/counter words. These
share the multiplier with90..94; later requests can raise an in-flight gain. Phase2
commits actual SPU depth0, closes SEQ, queues `CdFadeOut1` (actual command3) and starts
its original three-update countdown. Completion performs real all24-voice RR8/key-off
and current script/allocation cleanup before clearing the fade flag and advancing.
Paused script copies, pending effects, voice envelopes/decoders and shared SPU RAM
remain retained. No replacement shutdown timer or voice pool is used.

Original StopSeq releases all24 voices when an actual SEQ access is open. A natural
event-stream end releases its own notes and retains the access until explicit close;
it must not silently clear unrelated effects. RR-only SDK writes clear low6 ADSR
bits, including exponential release, while keeping the other bits. CdFadeOut1 uses
the existing real CD fade/pause/retirement and restores both master words to127.
With no playing identity it restores them before advancing. Stop2 keeps its separate
behavior. Native final mixing applies the committed master gain once after actual
SPU/CD and any legacy contributions, retaining the current native clipping/gain
convention. Physical console mixer equivalence remains unproven.

Public authored tests cover all five profiles in20 playing/no-playing and open/no-SEQ
contexts, exact phase/countdown/release boundaries, rejected unbound requests,
retained pending/copy owners and stop3 restoration. Debug/ASan traces match50 contexts,
45,000 updates and1,415 queued calls byte for byte. Original US instruction execution
matches both full queues, signed master/counter/restore words, SEQ access, all24 actual
SDK release registers and both actual SDK master-volume MMIO registers. Original
SDK RR/master functions execute; CD hardware, VSync, serial/CD volume, reverb depth
and SEQ stop/close services have explicit adapters. Supplied SEQ-access/register
and expiry-clock fixtures isolate those boundaries and do not establish a route.
Five additional contexts/4,500 updates/1,280 calls match original queue overflow
that discards command3. Master gain stays zero when the actual reset command is lost;
no out-of-band restoration is invented.

Actual native Debug/ASan final-mix checks each cover1,061 frames, five all24 release callbacks,
two actual private Library SEQ closures,964 partial-gain and45 zero-gain frames.
All final PCM samples match a single committed master multiplication;1,468,443
samples differ from bypassing it. Eight menu updates hold player/game time while
audio advances. A real five-MP Summon cast has34 frozen/release audio updates, kills
a real enemy and earns level2 (HP71/MP24) after the initial70HP/20MP and cast-to15MP.
Twenty-five repeated draws hold all audio clocks. Arrival/card/load/protection,
stage-event and VAB-tone fixtures retain their separate scope. All six final
Debug/ASan CD, rescue and actual-app regressions pass with normal inferior exits.
These checks do not close bootstrap/game-over routing, global sound-ring ordering, CD sends,
session continuity, ordinary familiar activation/persistence or whole-pass gates.

Selected bound native SPU controls now share the actual sound-command ring with
XA/master commands:7 closes SEQ, E/F pause/resume scripts, A1 releases20/21 at RR14,
and202/204 start the attached private Library/Chapel SEQ. The native owner queues
stage sequence requests and spell pause/release rather than executing them early.
Effects and scripts update before command execution. A duplicate XA early return
therefore holds later SPU commands until the next command pass. A resumed cursor
starts its next script update on the following audio frame. Captured render lease
spans still precede SEQ commands; commands themselves add no DSP update.
The script allocation/update gate follows the retained DRA SEQ access, including
a natural end of playback. Only explicit close returns it to ordinary channels.

These calls require the actual configured borrowed sound dependency. Unsupported
commands or absent binding reject before enqueueing; attached SEQ validation still
belongs to its actual owner at execution. Existing standalone and unbound native
sound APIs retain their selected behavior. Initial bootstrap, game-over routing,
complete command/effect-ring ordering and session continuity remain unfinished.
The shared RR helper now also clears the exponential-release bit on A1, matching
the actual SDK RR-only write. Original InitSoundVars2 clears the pause flag while
retaining copied scripts and pending effects; all-voice cleanup now preserves that
distinction as well. The DRA SEQ gate remains active after SDK end-of-track;
allocation and updates use the retained access until explicit close. A public
regression now starts a real authored SEQ through its actual sound API, ends it,
checks the retained ordinary lease/fourth span and closes through StopSeq.

Public authored checks use actual sound construction, VABs and rendering for
post-update pause copies, delayed resume, SEQ start/close frame spans, duplicate-XA
holds, wrapped clocks, rejected unbound controls and exact20/21 release bits.
Debug/ASan command traces are identical across16 contexts,1,280 updates and2,136
calls. Original instructions match both256-entry queues, all live and copied
script words, pause flag, SEQ access, reverb/master words and all24 RR registers.
Actual SDK RR/master functions execute to MMIO. SDK SEQ Open/Play/SetVol/Stop/Close,
CD hardware, VSync, serial/CD volume and depth use explicit adapters. These
command-only contexts do not advance script or DSP scheduling; supplied cursor,
copy and register vectors remain component fixtures.

Actual Debug/ASan native checks each use a real private sound script, two post-script
pause/resume pairs, a duplicate XA holding E, deferred private Library SEQ births
and full SPU work-RAM/DSP holds through control execution. Their1,075 final mixed
frames also preserve all five master fades, actual menu/spell-freeze/draw clocks
and an earned level-up from the ordinary Summon cast. Actual sound queue, card,
load, protection, arrival, stage-event and tone fixtures remain separate from
ordinary producer/acquisition/traversal and console evidence. Final native/app
regressions and exact-head platform results accompany the batch before publication.
A separate authored-EOT native fixture checks the deferred birth, retained fourth
span after end, queued7 retaining that frame's span and ordinary span on the next
update, then restores the real private Library attachment. This fixture does not
establish an ordinary song's ending or original SDK scheduling parity.

The final corrected branch also matches4,608 complete original script-allocation
calls across all144 private script definitions without helper adapters. Those
fixtures include a stopped playback cursor with retained SEQ access; they compare
allocation/cursor/priority metadata and key-off masks, rather than actual playback.
All six final Debug/ASan CD, rescue and actual-app regressions pass on the corrected
binaries with normal inferior exits.


Bound [native game-over audio](laboratory-game-over.md) now supplies the selected
Engine81/Init11 producer frames, actual acknowledgement/XA wait,12/B cleanup,
MuteSound and33e source/resource phases. B executes stop2, StopSeq and real all24
RR8;12 owns CD16's abandoned-position reset. Mute validates actual closed SEQ and
retires current/saved/in-flight native decoders while resetting original queue and
current metadata. It advances no samples and retains SPU/DSP/RAM, copied scripts,
restore byte and original retained XA words. Actual bound game-over audio no longer
overlaps a legacy decoder or skips its live SPU owner. Source comparisons and actual
dying-player checks preserve their separate scopes in the linked document. Unbound
bootstrap/game-over and complete cleanup/global/session ownership remain open.


## Common native bootstrap and bank loading

Native sessions now initialize and bind this common CD owner after the actual
SPU/stream resources, before any familiar selection. Initial requests use the
existing verified private stage-profile identity; source MU_NO_AUDIO creates no
transport, and actual flag20 rooms omit the initial stage request. Command4 performs
the real configuration and ReadN before publishing playback. A present CD owner
does not imply an initialized familiar overlay, root, found card or bank2.

First familiar bank loading now prepares graphics/SPU resources, checks the sound
dependency without mutation and rebinds the same CD immediately before retiring
the previous SPU. Current/saved decoders, pending incoming/CD queues and actual
playback position survive. Voices, script cursors, effect read/write slots, DSP
phase and RAM outside the loaded bank retain their existing ownership. Failed
resource preparation preserves the live CD and complete SPU state. Cached and
connected familiar validation distinguishes global CD existence from actual
loaded overlay/root readiness.

Actual save-room entry queues stopA and retains the source music-stopped word.
After leaving, native audio repeats stopA and waits for actual XA activity
(playing identity or fade) to clear before requesting the stage profile. This fixes
a legacy decoder being recreated beside an already bound CD on room exit.
No cooldown substitutes for activity. This batch does not infer a clock-room stop
from save-room source or complete the engine's room-fade/player-hold phases.

The same bootstrap owner now handles native game-over without any Fairy card,
resource load or bank2 fixture, using the selected
[game-over lifecycle](laboratory-game-over.md). No native start grants a familiar
or changes HP/MP, relics, equipment or version-nine saves.

Private comparisons execute264 bounded original instruction blocks for
Engine_Init music selection/requests and Engine_3 stop/readiness/restart, using
the actual private stage table and actual xa_play_ck query. They cover stage65
wind/mode, room20/40 flags, fades and playing identities. PlaySfx is an explicit
request adapter; these are selected instruction blocks, not whole-engine calls.

Actual Debug/ASan native checks cover all11 stage profiles, real command4/read
acknowledgements or source silence, and a first bank2 load with actual Library
bookmark/speech and pending E/90 controls. The same CD and decoder/queue state
survive with voice/register/script/effect-ring/DSP and outside-bank RAM retention.
A failed resource-root fixture retains the complete live SPU. Actual save23/room11
edge fixtures cover60 silent updates and two restarts; the active case holds six
updates before XA readiness, without a legacy decoder or save input/write.
Separate actual no-card physical/water/silent-music deaths exercise capture,64
decay uploads, source game-over requests, retained SPU and real presentation/PCM.

All220 public tests pass in Debug, Release and macOS ASan/UBSan, alongside34
Python tests, repository policy, formatting, static SDL linkage and Release ZIP
CRC/private-input checks. All eight final Debug/ASan CD, rescue, card-context
game-over and actual medicine-app regressions pass with normal exits and no
sanitizer errors. The no-card death checks are separate additional native evidence.

All such stage/card/resource/protection/audio/edge/lethal/HP1-water-result fixtures
remain separate from ordinary acquisition, traversal, full scene/room-fade
initialization, hardware timing and console evidence. Cross-session CD/SEQ/SPU
RAM/DSP continuity, changed-base retargeting, remaining global controls/cutscenes,
voice arbitration/key timing, CD wet sends and whole-pass acceptance remain open.
Ordinary Fairy menu activation and all six Alucard milestone gates remain open.
