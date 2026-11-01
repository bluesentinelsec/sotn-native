# Fairy item effects and speech

The US Fairy's DF item-particle and DD Life Apple light callbacks now have bounded
components using private profiles and SDK geometry. The existing XA decoder also
plays the eight original Fairy narrative clips. These are dependencies for native
Fairy ownership. They do not activate a familiar, reserve native slots or rows,
consume an item, restore resources or complete player17. Ordinary grants and savev9
stay unchanged; all six [Alucard gates](alucard-status.md) remain open.

DF reads its eleven private profiles and requests the original count plus one
sentinel:129 rows for profile0 or33 for the other profiles. The caller must reserve
those rows from the actual shared stage pool before birth. Exhaustion retires
before RNG. Particle origins use whole words while the entity retains fractions.
Each style preserves its original RNG order, birth activation/motion fallthrough,
signed counters, damping, gravity, fading and retirement. Profile0 activates one
particle per callback; its full128-particle initialization has already consumed
the original RNG calls. The source comment suggesting repeated Hammer activation
does not describe the executable behavior.

Raster coordinates rebind the fixed-point scratch high words before each actual
update, retaining fractions. Camera/room translations follow the original packet
flags without ticking or advancing RNG. The real sentinel remains leased, resets
its zero-width raster packet every callback and retires with the effect. There is
no second particle pool.

DD requests ten real untextured G4s. Birth retains source fractions, wraps the X±4
word and constructs eight fan quads from the private SDK sine/cosine tables. The
two columns and birth-only fan preserve their distinct geometry,435 draw mode,
priority448, color narrowing and signed counters. Its natural lifetime is186
callbacks; retirement precedes the last packet update. The ordinary fan packets
omit camera flag2, so translating the entity must not translate the fan again.
Actual room cleanup must still honor the entity's missing retention flag20000.
This light changes no HP, MP, inventory or scene state.

`SotnMusic_CreateSpeech` accepts the original selected Fairy narrative IDs472,
473,476,479,47A,48A,48E and492. They use the existing bounded4-bit stereo37.8kHz
XA decoder and finite playback cursor. Completion holds silence without reading
another clip. Short bank2 VAB voices remain separate. Decoder completion does not
supply an original CD-command queue or acknowledgement; those owners remain
required by [Fairy sound-state waits](fairy-controller.md).
The subsequent [CD audio service](familiar-cd-audio.md) supplies selected native
queues and decoder-backed operations; actual Fairy/engine binding remains open.

Public authored tests cover allocation-before-RNG,129-row capacity/sentinel,
birth motion and lifetimes, raster/fraction translations, invalid-cursor rollback,
DD geometry/counters/retained retirement packets, signed-word origin wrap and
finite XA endings. Private comparisons execute9,784 complete original DF/DD
callbacks in288 contexts, comparing477,552 complete item tiles including sentinels
and22,320 complete G4s. All eleven actual profiles, both facings, six fractional/
word-origin fixtures and optional allocation outcomes are covered. Allocator,
BIOS RNG and destruction adapters remain explicit; actual native leases and
retirement are separate work. Debug and ASan traces are byte-identical.

Native decoder checks play all eight private clips for their1,321 declared ticks,
produce1,805,038 nonzero PCM samples and then hold silence, in Debug and ASan/UBSan.
These checks use the existing SDL rate converter; original CD interpolation,
hardware timing and console parity are not established. Supplied origins,
allocation outcomes and direct clip checks are not ordinary familiar activation,
acquisition, rescue, traversal or console evidence.

All219 authored tests pass in Debug, Release and macOS ASan/UBSan, along with34
Python tests, repository policy, formatting, static SDL linkage and Release ZIP
checks. Both actual CMake-built apps pass the owned-medicine regression through
real presentation/resources, menu/draw holds and retirement. These regressions
preserve existing ownership; they do not establish native Fairy availability.

Provenance: `servant/tt_002/faerie.c:UpdateSubEntityUseItem/UpdateSubEntityUseLifeApple`
and `dra/sound.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later). The existing XA
decoder retains its separately documented PSY-Z provenance. Profiles, speech,
SDK tables and all other game data remain private.
