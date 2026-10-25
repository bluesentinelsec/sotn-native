# Library secret bookcase

The original rotating bookcase now blocks and yields to normal Alucard movement.
Contact uses the previous simulation tick's projected front/back edges. The first
discovery requires 96 contact ticks; subsequent pushes use the retained castle
flag. Original force, damping, signed angle wrap, side-on gaps and positional
stone sounds advance with simulation time. Static neighboring bookcases use their
private angles/depths and do not push the player.

The discovery flag is retained in existing v8 ordinary saves. Rotation, momentum
and the contact countdown remain transient: a fresh actor begins at its original
pose. No save-format or application-version change. Saved discovery is validated
as a boolean.

Private model/normal pointers, face indices, color matrix and static poses remain
in the user's LIB overlay. Private SDK trigonometry and reciprocal-root bytes
remain in the verified boot image. The bounded transform retains SDK projection,
lighting saturation, fog, backface culling and one/two levels of rounded quad
subdivision. It rejects malformed pointers, unsupported model/normal domains,
excess depth and invalid controls. The normal actor also presents its original
blended shadow.

## Validation and remaining scope

Public synthetic tests cover projection boundaries, malformed pointers/normals,
unchanged results on failure, subdivision budgets, both collision sides, initial
push delay, angular damping, side-on gaps, static variants, fresh-actor discovery
and invalid saved flags. Fixtures contain no original model or math lookup tables.

A private comparison matches 131072 projected/lit vertices against the pinned
SDK backend with original private math data. The comparison supplies the original
MIT SDK `SetFogNear` implementation because the pinned backend leaves it
unimplemented. It also uses the original executable's NCDS instruction
`0x4ae80413` (`sf=1`, `lm=1`); the backend's byte-reversed lighting literal decodes
those fields incorrectly. These explicit corrections are part of the comparison,
not evidence of full console GPU parity.

A native LLDB check uses the ordinary development character and original room-1
placements. Holding right discovers the passage on tick 111 and crosses it on
tick 190, with 48 rendered frames. An explicit transfer to room 14 then exercises
the ordinary save-room UI in an isolated test directory and fresh Continue,
retaining discovery. Debug and sanitizer runs are checked separately. This is
not an uninterrupted castle route or a console replay comparison.

Complete GPU dithering/rasterization, full shop panels and optional services,
sitting, Mist-form interaction and whole-route parity remain incomplete.
[Selected connected travel](library-connections.md) has separate validation.
Do not describe the Library or game as complete.

Behavior and subdivision adapt `st/lib/first_c_file.c:func_us_801AE8E8` and
`st/e_misc.h:UnkRecursivePrimFunc2` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Selected fixed lighting/projection adapts PSY-Z
`b52003e55c0d1b20719e984dd19bd4d760eeb14a` (MPL-2.0); fog follows the pinned
`src/main/psxsdk/libgte/fog_01.c` (MIT). Original runtime tables stay private.
