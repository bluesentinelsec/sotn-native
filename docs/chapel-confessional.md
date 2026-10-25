# Chapel confessional and sitting

The internal Royal Chapel profile now runs its placed chairs, priest/parishioner
encounters, curtains, blades and debris. Up alone near a chair starts original
alignment and sitting; normal directional, jump and hand-attack controls release
Alucard. The player keeps his fractional position during whole-pixel alignment.
The two original sitting animations and facing rules come from private DRA.

The original F2 sitting pose requests a floating sleep letter after 6,144 ticks,
then every 512 ticks. Its motion and four-frame animation borrow private DRA and
common artwork. Letters disappear on leaving the seated state or changing rooms.
The selected native player owns one transient letter slot; its short lifetime
cannot overlap the original factory interval. The currently integrated Chapel
chairs do not request the separate Outer Wall snore.

## Encounter behavior

Each confessional chair creates its opposite ghost in the original adjacent slot.
Native slot/generation checks protect unrelated occupants and expired children.
The owned original actor RNG selects good/bad variants. Private stage profiles
supply animation pointers, curtain bounds, blade positions and signed movement.
The priest's apparent `FIX(0.75)` initializer narrows to a signed 16-bit value in
the US profile: preserve the resulting quarter-pixel step left. Do not replace it
with a preferred full-width interpretation.

Ghosts wait for completed sitting, settle to the original floor sensors, fade in
and approach. The good priest waits 256 ticks, fades away and has the original
one-in-four Grape Juice drop. Leaving early suppresses that reward. A good
parishioner leaves when Alucard stands. Bad ghosts close and shake the curtain,
then emit four priest blades or one parishioner blade at original intervals.
Blades extend, wait 96 ticks and retract; normal attacks destroy them, and contact
uses the existing physical-damage service. Selected debris retains original
random motion, gravity and rotation. Optional actor/primitive exhaustion omits
presentation rather than overwriting another owner or preventing ordinary sitting.
Unsigned-byte fade wrap and the completed ghost's dormant state remain explicit.

The original `pauseEnemies` flag blocks new stopwatch use and retires an existing
clock; it does not freeze ordinary actors indiscriminately. It resets on room
entry. The confessional's transient chime flag survives room changes within the
same stage owner, and is not part of version-eight ordinary saves.

## Audio and boundaries

`ST/DAI/SEQ_DAI.SEQ` is an integrity-checked private resource. Only its filename,
length and fingerprint enter the public manifest. The sound service attaches the
original Chapel identity 0x204 separately from Library 0x202 and reads that track's
original volume, bank and repeat configuration. Invalid replacement inputs retain
the current owner. Events run through the existing selected SEQ/SPU service.

The parishioner's initialization starts the original chime and fades XA stage
music using the original integer volume equation. The placed tower end-chime
stops the sequence and normally restarts the original Chapel track. Preserve the
US trigger's unusual already-playing-XA branch: it issues the slower sound-master
fade and remains dormant. Native stream stop replaces the subsequent CD command;
full CD queue/busy/release-tail timing, reverb and console PCM parity remain
incomplete. Sequence and XA ownership remain independent.

This is an internal Chapel development checkpoint. Maria, Hippogryph, connected
Chapel travel, whole-stage replay and complete audiovisual/timing parity remain
incomplete. No public start mode, enhancement, version or save-format change is
introduced. Existing user slots stay intact.

## Validation

Public synthetic tests cover chair alignment/facing, Up gating, seated controls,
private animation completion, sleep timing/motion, all four ghost variants,
curtain/blade timing, reward/early departure, interrupted fade wrap, debris,
optional allocation exhaustion, ownership, invalid data, chime branches,
stopwatch cancellation and independent stage sequence identities. Tests use
synthetic inputs, never original runtime arrays or artwork.

Private native LLDB checks use original placed chairs and normal Up controls for
all four ghost branches, with explicit room/variant and selected reward-RNG
fixtures. They check original blade contact and the priest reward. A separate
3,600-tick chime check produces nonzero PCM and verifies independent XA ownership;
a controlled same-owner room rebase reaches the placed tower end-chime and checks
stage music restoration. Captures remain private. These checks do not establish
ordinary uninterrupted traversal, listening quality or console parity.

All 94 public groups pass in Debug, macOS ASan/UBSan and Release; release
packaging passes. Private Debug and sanitizer runs pass 2,173 encounter ticks
with 543 regular renders and a separate 6,800-tick sitting check: two letters
change 36 rendered pixels. A further 20,000-tick private chime run traverses five
rewinds and produces identical Debug/sanitizer PCM hashes. Both native fade
equations, setup ticks and independent owners pass controlled checks.

Shared private regressions pass four Chapel tower crossings and three
Wall/Library connectors, isolated save/fresh Continue and failed-destination
owner retention. No existing user save slot is written by these fixtures.
