# Shared familiar resource and animation foundation

The five US familiars now have fixed, verified private resource IDs. Their common
animation helper, original stage-bank20 sprite binding and bank2 sound binding
have bounded native services. This foundation does not activate a familiar or
supply its slots4..7, behavior, combat, item use or progression. Fairy Life Apple
rescue17 and Sword Brothers39 remain unavailable. Ordinary starts, grants and
savev9 are unchanged.

Each family loads its own TT overlay, FT textures and VAB header/body through the
existing on-demand resource loader. Public metadata contains filenames, lengths
and revision fingerprints, never game bytes. Views are immutable, and overlay
addresses are private data references; no PS1 code executes as host code. The
caller retains borrowed overlay/header bytes through replacement or destruction.

`SotnFamiliarAnimation` preserves the original signed-word timer and unsigned pose,
conditional pointer-identity restart, explicit unconditional restart, single loop
jump, timeline transfer and end hold. A zero timer loads the current duration
without advancing the pose. Existing signed16 hitbox offsets survive when the
optional frame-property source is absent. Late bounds failures preserve both
cursor and result. Callers still own the actual update clock and entity lifecycle;
drawing must never advance this cursor.

The renderer binds the original overlay sprite bank20 to existing resident VRAM.
FT uploads occupy64x128 words at2C0/100 and32x128 at2C0/180. The palette copies all
256 original words to0/F4, including adjacent private bytes beyond the source's
64-word Fairy declaration. It never reads beyond the bounded overlay. Sprite
tables may contain unused null entries; these remain unavailable draw indices.
All nonnull entries validate before any upload or binding change. Clearing the
borrowed binding leaves the resident pixels intact. No alternate artwork memory,
entity pool or primitive capacity is introduced.

The sound service binds the actual VAB to its existing bank2 and private original
SPU address. Validation completes before resident sample bytes or bank metadata
change. Existing voices, scripts and queues retain their clocks and leases;
other bank metadata stays unchanged. A bank binding alone does not supply the
Fairy's event/audio handshake or full SPU timing parity.

Authored public tests cover animation transfer/hold/single-jump behavior, signed
timer wrap, pointer aliases, retained16-bit offsets, optional/malformed properties
and rollback. Renderer tests cover actual bank20 gating, usable/null frame entries,
late bad pointers, unchanged rendered pixels after failure and resident pixels
after clearing. Sound tests bind during a playing voice, preserve other owners,
reject late malformed sample sizes atomically and key a real bank2 voice.

Private original-instruction comparisons match736 contexts and39,842 complete
`ServantUpdateAnim` calls across all46 private Fairy timelines, natural playback
and timer boundaries, with optional authored frame-property bytes. They use no
service adapters. Debug and ASan traces are identical. These component comparisons
do not establish native familiar availability.

Separate muted native checks load all20 genuine US resources, validate the five
logical groups and exact FT lengths, bind all five real VABs, retain a ringing
Alucard voice and all other bank/script/queue state, and synthesize actual Fairy
regeneration into nonzero PCM. They open no audio device. Renderer checks copy
exactly12,544 resident words, preserve all unrelated VRAM words/validity, render
250 usable Fairy frame/facing combinations with65,418 nonzero pixels, hold memory
on repeated draws and reject a late final-table pointer without changing any
binding or VRAM. Debug and ASan checks pass with normal exits. The initial loader
rejected the real null sentinel; the corrected loader preserves that unavailable
index, and authored tests cover the distinction.

All216 authored tests pass in Debug, Release and macOS ASan/UBSan; the actual
CMake-built medicine regression passes in both native app builds. Python tests,
repository policy, formatting, static SDL linkage and Release packaging pass.

Explicit voice, sprite, facing and frame-property fixtures are not actual familiar
entities, ordinary acquisition, uninterrupted routes or console/GPU comparisons.
Actual initialization/selection, slots, movement, room events, item effects,
rescue scene ownership, combat/progression and shared contention still require
native owners. All six [Alucard gates](alucard-status.md) remain open.
The subsequent [Fairy control components](fairy-controller.md) reconstruct selected
callbacks and supply actual native player inputs; they do not close those gates.

Provenance: `servant/servant_update_anim.h`,
`servant/tt_002/faerie.c:SetAnimationFrame/ServantInit`, `dra/cd.c` familiar uploads,
`dra/4B758.c` sprite binding and the existing `dra/sound.c` VAB service, pinned
sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Private tables, artwork, samples and raw comparison traces are not published.
