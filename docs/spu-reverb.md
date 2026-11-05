# Selected native SPU voice reverb

Selected native Alucard sound now initializes the original Studio B voice-reverb configuration from
the verified private US executable. Its32 coefficients remain runtime data. The
processor uses the existing512KB SPU RAM and original work address, not another
sample/effect pool. Initialization validates every loaded VAB before clearing only
the work area; subsequent uploads cannot overwrite that reservation.

Original `SoundKey` tone mode mask0x4 selects each actual voice's send. All24 voices
retain their existing decoding, envelope, panning and allocation owners. Dry tones
remain dry. Reverb runs at the original half rate with retained44.1kHz input/output
phase, IIR/comb/APF work, signed arithmetic, stereo feedback and real wrapping work
cursor. Disabling writes retains reads and cursor motion. Zero depth mutes the wet
output while processing continues. Signed SDK depth conversion retains word wrap.

Native initialization applies original depth10. Selected Library/Chapel SEQ playback
now applies its actual private sequence depth before publication. Standalone sound
callers need the real configuration before this processor is available. Native
initialization fails on malformed configuration or overlapping actual sample RAM.
Repeated initialization cannot silently reset a live processor.

Actual sound updates advance the processor through menus and spell freeze while
player/game clocks retain their existing holds. Drawing never advances it. Preparing
a familiar VAB copies the actual work RAM, phase, cursor and feedback with the
existing sound owner; a failed upload preserves those live owners. Ordinary grants,
HP/MP defaults, familiar activation gates and v9 saves are unchanged.

The CPU processor adapts unmodified PSY-Z revision
`b52003e55c0d1b20719e984dd19bd4d760eeb14a`,
`psyz/src/psyz/psyz_spu.c`. SDK configuration, tone sends and depth conversion follow
the corresponding original SDK paths documented in [third-party notices](../THIRD_PARTY.md).
Public authored checks cover impulse/stereo response, signed depth, continuing
processing at zero depth, write holds, wrapping, unrelated RAM preservation,
actual voice routing and familiar upload failure/retention.

Private Debug/ASan comparisons execute the full unchanged PSY-Z reverb functions
through explicit register/RAM/input adapters. Eight contexts cover524,288 samples,
comparing every output, phase, cursor, retained input/feedback and complete work RAM;
420,695 stereo samples are nonzero. Separate original US SDK instruction checks
execute19 complete calls, matching the work address, all32 hardware coefficient
registers, enable bit and17 signed stereo depth pairs. Their MMIO/allocation adapters
do not establish physical SPU timing or console audio equivalence.

Actual native audio checks configure all11 current stage-bank profiles and selected
Library/Chapel SEQ depth/playback. Real VAB tone fixtures produce473,418 wet-versus-zero-depth
sample differences and516,812 nonzero samples while voice decoding and complete RAM
remain identical to the reference at zero depth. Supplied tone keys are audio-service
evidence, separate from an ordinary effect producer or acquisition route.

Both native scene checks pass1,022 production audio callbacks,677 audible wet
updates and993,453 sample differences with an explicit real-tone send fixture.
They preserve actual familiar-load DSP state without a tick, eight menu holds,
a real five-MP Summon cast and34 frozen/release audio updates, and five repeated
draw holds. Default70HP/20MP precede the actual cast to15MP. Card/load/protection/tone
fixtures do not establish ordinary familiar selection, acquisition or traversal.

Selected bound CD mode0 expiry now applies actual depth127 before command6, whose
completion restores the current original `g_ReverbDepth` byte. The byte starts at
BSS zero independently of SoundInit's current SDK depth10; real SEQ playback updates
it. Prepared sound replacement rebinds the CD dependency before retiring the old
owner. Neither binding nor these gain operations advance DSP samples.

Native audio now updates commands/scripts/SEQ, executes CD work, then renders the
actual voice/DSP samples. The retained command-frame snapshot preserves the script
lease span selected before SEQ completion, including the fourth-channel boundary.
Existing `SotnSound_Tick` callers retain the combined update/render service.

Original-instruction comparisons now also match both actual signed SPU depth words
and the restore byte through95,400 updates and both full command rings. Separate
normal playback passes14,470 unskipped command/CD/render updates with BSS-zero and
actual Library SEQ restore depths. Supplied engine-expiry and sound-replacement
fixtures additionally verify same-frame native PCM and binding lifetime; they do
not establish an uninterrupted gameplay route.

Bound master80..84 now commit depth0 before same-frame rendering, actual SEQ
closure, delayed all24-voice release and final native master gain. See the
[CD owner](familiar-cd-audio.md) for source/SDK/native evidence and boundaries.

CD reverb sends, other presets, remaining mixer/SDK
scheduling and cross-session DSP continuity remain open.
This voice processor does not complete [shared CD audio](familiar-cd-audio.md),
ordinary familiar activation, Alucard, castle or console parity. All six
[milestone gates](alucard-status.md) remain open.
