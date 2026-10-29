# Spell sound-script pause and resume

The native sound service now accepts original queued commands0xE/0xF. They pause
and resume eligible sound-script cursors in queue order, while already playing
voices, direct effects, music and the audio device continue. Script mode5 is exempt.
The [selected native spirit integration](player-spirit-spells.md) now connects
these commands at actual scene-freeze entry/release edges.

The actual sound owner retains four original channels and four copied cursor
slots. Pause stores each eligible running cursor and clears its live identity,
address, timer, volume, distance, mode and allocation/priority state. It does not
key off its voices. Repeated pause commands retain old copies when no eligible
cursor replaces them. New sounds can still start while the pause flag is set.

Resume scans copied channels0..2 in order and restores into the first free
allocation channel. It preserves the original distinction between a running cursor
and its allocation identity: restore does not reserve the channel or restore its
allocation priority. Multiple copied cursors can consequently restore over that
same free channel. A copied cursor with no free ordinary destination is discarded;
a blocked fourth-channel copy remains available for a later resume. These original
behaviors are retained, including their allocation-pressure results.

Script startup now uses the real allocation identity while an old voice tail is
still playing. It checks a matching allocation, then a free slot, then strictly
lower priorities before equal priorities. An active stage sequence uses the fourth
script channel. Original startup for that channel keys off12..15, while virtual
voice33 plays in14..17; those ranges intentionally remain distinct. Script cursor
termination releases its allocation only when the selected native envelopes are
off. Full SDK voice pressure, SEQ transitions, envelope/command timing, reverb and
SPU/audio parity remain incomplete; this does not establish whole-audio fidelity.

Public authored PCM/state tests cover command ordering, non-pausing scripts,
continued voices, repeated commands, old copies, new sounds during pause,
restore overwrite/drop, blocked fourth-channel recovery, retained voice tails,
strict-before-equal priority, fourth-channel playback and invalid volume/pan
commands. They create no audio device. Private comparisons match2,048 complete
original pause/resume/restore instruction contexts and4,608 complete allocation/
constructor contexts across all144 private script definitions. They include exact
key-off masks, gains, distance, modes, cursor metadata and both sequence states;
no helper adapters substitute those routines. Debug and ASan/UBSan traces match.
A separate muted native Dark Metamorphosis regression retains the documented
protected equipment/position/Bloodstone fixtures from its existing checkpoint.
It verifies existing combat/healing/effect/sound paths; it is not a spirit-freeze,
complete sound, uninterrupted-route or console replay. No ordinary saves are written.

Ordinary audio defaults and version-nine saves remain unchanged. The selected
native owner now connects these commands to real [spell-freeze edges](spirit-casting.md)
and [frozen-stage dispatch](stage-spell-freeze.md).

Provenance: selected `dra/sound.c:{PauseSfxScripts,UnpauseSfxScripts,
RestoreSfxScriptData,PlaySfxScript,func_8013572C,func_80135D8C,func_80136010}`,
pinned sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`,
AGPL-3.0-or-later. All sound data, original images and traces remain private.
