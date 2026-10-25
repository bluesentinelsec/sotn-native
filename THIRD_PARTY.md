# Third-party components

- **SDL 3.4.18**, zlib license. Downloaded from the official release archive by
  CMake FetchContent, with SHA-256 verification. The source remains in the
  ignored build tree. Packages include SDL's license under `licenses/SDL3.txt`.
  Source: https://github.com/libsdl-org/SDL/releases/tag/release-3.4.18
  On macOS, `cmake/PatchSDLAudio.cmake` applies an original, hash-guarded playback
  shutdown adjustment. It avoids scheduling a redundant asynchronous stop before
  queue disposal, which reproduced a CoreAudio `-66671` warning under LLDB.
  Recording and other platforms retain the release implementation.
- **Gradle Wrapper 8.10.2**, Apache License 2.0. The wrapper scripts and JAR are
  build tooling from Gradle. The distribution is pinned with its official
  SHA-256 in `platforms/android/gradle/wrapper/gradle-wrapper.properties`.
  Source and license: https://github.com/gradle/gradle/tree/v8.10.2

- **Reconstructed SoTN game algorithms**, AGPL-3.0-or-later, adapted from
  sotn-decomp commit `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
  `modules/core/collision.c` derives from `src/dra/collider.c`;
  `modules/core/decompress.c` derives from the graphics decoder in
  `src/dra/4AEA4.c`. `modules/core/animation.c` adapts `src/dra/alu_anim.c`.
  `modules/core/player.c` adapts normal-form movement paths from
  `src/dra/6DF70.c`, `6E42C.c`, `6FDF8.c`, `71830.c`, and `692E8.c`;
  `modules/core/player_sensors.c` adapts `6BF64.c`, `70394.c`, and the
  normal-form collision path in `692E8.c`. See
  [movement integration](docs/laboratory-movement.md) for scope and validation.
  `modules/core/room_actors.c` adapts scrolling spawns from `src/st/create_entity.h`
  and stage persistence from `src/st/st_common.h`; the runtime layout parser
  is original project code. See [actor integration](docs/laboratory-actors.md).
  `modules/render/actor_graphics.c` adapts bounded graphics-bank uploads,
  bulk palettes, and multipart drawing from `src/dra/4AEA4.c`, `4A538.c`,
  and `4B758.c`.
  `modules/core/actors.c` adapts selected intact object and active Bloody Zombie
  paths from `src/st/nz0/e_breakable.c`, `e_red_eye_bust.c`, `e_nz0_room2.c`,
  `src/st/e_bloody_zombie.h`, `animate_entity.h`, `st_common.h`, and `st_update.h`.
  `modules/render/tilemap.c` adapts the tile renderer in `src/dra/4CE2C.c` and
  tile upload layout in `src/pc/sim_pc.c`. `modules/render/alucard_sprite.c`
  adapts the sprite-sheet path in `src/dra/4B758.c` and ARC pointer loading in
  `src/pc/pl_arc.c`. Each file identifies its origin. Adaptations are described
  in [game-core integration](docs/game-core.md) and
  [runtime presentation](docs/laboratory-presentation.md). Packages include the
  [full license](licenses/AGPL-3.0-or-later.txt); binaries linking this code are
  subject to AGPL requirements, including corresponding source availability.
  Source: https://github.com/Xeeynamo/sotn-decomp/tree/7bc54d8157b6bed77e6bd15e5db9461a734fe1e5

No original game source dumps, artwork, audio, disc images, BIOS images, or
extracted game data are included. The project license applies to original
scaffold code; adapted components retain their stated licenses.

## Testing and private comparisons

- **mesa-dist-win 26.2.4 MSVC release**, used only for Windows CI's software
  OpenGL framebuffer check. Downloaded into ignored `build/test-tools/mesa`
  with SHA-256 `351fc8c8b695878ffb3eaa044b3ead08672a48b1a045e3c3e3975811df0f6695`.
  No driver binaries are tracked, installed system-wide, or packaged. The
  archive retains its upstream notices. Source and distribution:
  https://github.com/pal1000/mesa-dist-win/releases/tag/26.2.4
- `tools/patches/psyz-macos-opengl.patch` contains an original build adjustment
  for PSY-Z commit `b52003e55c0d1b20719e984dd19bd4d760eeb14a`, used only in the
  ignored upstream comparison checkout. It selects existing upstream files;
  it does not import those files or their dependencies into this app. PSY-Z's
  license inventory lists MPL 2.0 platform code, MIT decomp code in its own
  directory, and unlicensed header sections. Resolve each proposed import's
  provenance before integration. Inventory:
  https://github.com/Xeeynamo/psyz/blob/b52003e55c0d1b20719e984dd19bd4d760eeb14a/LICENSE

The empty-hand player dependency additionally adapts the selected
`EntityUnarmedAttack` and `UpdateUnarmedAnim` paths in `src/dra/7879C.c` and
`alu_anim.c` at the same pinned reconstructed C revision, under
AGPL-3.0-or-later. Their animation/hitbox tables remain private runtime data.

The selected Laboratory combat paths additionally adapt `src/st/collision.h`,
`e_collect.h`, `collect_subweapon.h`, and `src/dra/5D5BC.c` (`DealDamage`),
including original physical player-hit paths in `71830.c`. They retain
AGPL-3.0-or-later attribution at the same pinned revision. Game tables and
sound samples are not included.

## Selected native sound service

`modules/audio/sound.c` adapts selected original event/script/channel paths in
`src/dra/sound.c` under AGPL-3.0-or-later at the pinned game reconstruction.
Its pitch/volume service follows PSY-Z's MIT-licensed `decomp/src/libsnd/vm_n2p.c`
and `vm_nowon.c`; see `licenses/PSY-Z-decomp-MIT.txt`.

`modules/audio/voice.h` adapts selected decoding/resampling/envelope code from
[PSY-Z](https://github.com/Xeeynamo/psyz) revision
`b52003e55c0d1b20719e984dd19bd4d760eeb14a`,
`psyz/src/psyz/spu_voice.h`, under MPL-2.0. The full license ships as
`licenses/MPL-2.0.txt`. `gauss.h` retains the source's public-domain declaration
for its mathematical coefficients. No SDK/platform headers, original samples,
or game instrument/event tables are included.

`modules/audio/music.c` additionally adapts selected XA decoding paths in
PSY-Z's `psyz/src/psyz/libcd.c` at the same backend revision under MPL-2.0.
The native stream loader and SDL rate-conversion boundary replace platform/SDK
services. No game music, CD sectors, or soundtrack metadata arrays are included.

Selected Gladius/Mace integration additionally adapts `src/weapon/w_000.c`,
`w_002.c`, `shared.h`, and selected `src/dra/menu.c:CalcAttack` and
`5D5BC.c:GetEquipProperties` paths at the pinned reconstruction revision under
AGPL-3.0-or-later. Equipment records, weapon overlays, frame/hitbox tables,
palettes, and pixels remain private runtime inputs.

Original HUD presentation adapts selected `src/dra/5F60C.c` HUD/counter paths
and `4A538.c` palette behavior at the same pinned reconstruction revision under
AGPL-3.0-or-later. Layout, glyphs, artwork, palettes, and animation definitions
remain private runtime data; none are compiled into public binaries.

Selected physical death and game-over presentation additionally adapt
`src/dra/71830.c:PlayerStepKill` and `game_handlers.c:HandleGameOver`/
`func_800E5358` at the pinned reconstruction revision under AGPL-3.0-or-later.
Game-over bitmaps, palettes, animation frames, and music remain private inputs.

Selected save-point activation/centering/coffin clocks and mesh presentation adapt
`src/dra/62DEC.c`, `6DF70.c:func_8010E334`, and `menu.c:MenuDrawChar` at the same
pinned reconstruction revision under AGPL-3.0-or-later. Meshes, UV arrays, font,
texture pixels, and palette records remain private runtime inputs. Native storage
and slot-selection adapters are original project code. See
[save-room scope](docs/laboratory-save-rooms.md) for parity and platform limits.

`modules/core/character.c` adapts selected inventory acquisition/order, equipment
exchange, and unbuffed attribute calculations from `src/dra/5D5BC.c` and `menu.c`
at the pinned reconstruction revision under AGPL-3.0-or-later. Item records,
names, descriptions, and icons remain private runtime inputs. Unsupported item
behaviors are rejected at the equipment boundary.

Selected native equipment/pause panels, fonts, portrait, and icon loading adapt
`src/dra/menu.c` and `4AEA4.c`; cape palette selection adapts `5F60C.c`. Original
equipment drops, persistent room pickups, and breakable globe paths additionally
adapt `src/st/e_collect.h`, `nz0/e_nz0_room2.c`, and `st_common.h`. Basilard adapts
selected `src/weapon/w_031.c` behavior. All use the same pinned reconstruction
under AGPL-3.0-or-later. Private runtime definitions/pixels/palettes remain untracked.

Character progression additionally adapts `5D5BC.c:CheckAndDoLevelUp` and
`func_800FE044`, `5F60C.c` MP regeneration, and `e_collect.h` Heart/Life Max Up
collection at the same AGPL-3.0-or-later reconstruction. Experience/HP growth
and enemy reward tables remain private runtime inputs.

The additional native Laboratory enemy runtime in `modules/core/enemy_skeleton.c`,
`enemy_knight.c`, `enemy_spittle.c`, and the shared actor collision services adapts
reviewed algorithms from `src/st/e_skeleton.h`, `e_bone_scimitar.h`,
`e_blood_skeleton.h`, `entity_axe_knight.h`, `st_common.h`, `e_misc.h`, and
`src/st/nz0/e_spittle_bone.c` at the pinned reconstructed revision, under
AGPL-3.0-or-later. Runtime definitions/arrays and artwork remain private inputs.

The Laboratory room mechanism slice additionally adapts reviewed logic from
`src/st/nz0/e_nz0_room2.c`, `e_puzzle.c`, `e_elevator.c`, `e_elevator2.c`,
`e_cannon.c`, `e_left_secret_room_wall.c`, shared `st/e_red_door.h`,
`e_sealed_door.h`, `st_common.h`, `e_collect.h`, and packed visitation from
`src/dra/5087C.c`, at reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
All original tables, art, and patches remain private runtime input. Native bounds,
ownership, initialized puzzle sentinel, and tests are described in
[the traversal milestone](docs/laboratory-world.md).

Normal camera follow in `modules/core/camera.c` and bounded camera-lock behavior
in `actor_camera.c` adapt `src/dra/5087C.c:func_800F0CD8` and
`src/st/e_lock_camera.h` at that pinned reconstructed revision
(AGPL-3.0-or-later). Public tests use synthetic bounds and tables.

The paired Laboratory boss runtime adapts `src/st/nz0/slogra.c`, `gaibon.c`,
`bossfight.c`, and shared `st/e_life_up.h` / `e_explosion_puff_opaque.h` at
reconstructed revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`
(AGPL-3.0-or-later). Animation, hitbox, reward, art, audio, and SDK lookup tables
remain private runtime input. Native reward ownership packs the original drawable/data
primitive pairs into initialized C records.

`modules/core/game_math.c` follows the MIT-licensed reconstructed
`src/main/psxsdk/libgte/ratan.c`, `geo_00.c` and `geo_01.c` at that revision, using the user's boot image
for its quantized lookup. The [SDK/decomp MIT notice](licenses/PSY-Z-decomp-MIT.txt)
retains the reconstruction contributors' attribution. No SDK table is embedded.
`modules/core/boss_projection.c` adapts selected matrix, clipping, depth-ordering and UNR divider algorithms
from PSY-Z `b52003e55c0d1b20719e984dd19bd4d760eeb14a`,
`psyz/src/psyz/libgte.c` (MPL-2.0); the [MPL notice](licenses/MPL-2.0.txt) is packaged.
Its reciprocal estimates are generated mathematically. It supplies bounded reward, intro and Library book projection paths, not a general
PS1/GTE emulator. Book geometry, UVs and the original matrix-pair table remain private.

Exploration-map reveal, secret-passage, cursor, and orientation logic adapts
`src/dra/5087C.c` and `src/dra/624DC.c` at the pinned sotn-decomp revision above,
under AGPL-3.0-or-later. Blueprint, palette, and passage tables remain private.

Armor Lord behavior in `modules/core/enemy_armor*.c` adapts
`src/st/e_armor_lord.h` and selected `src/st/prim_helpers.h` logic at reconstructed
revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
The selected four-tick weapon-startup signal follows `src/dra/6E42C.c` and
`src/dra/692E8.c` at the same revision. The equipment element prepass in
`modules/core/combat.c` adapts
`src/dra/5D5BC.c:HandleDamage` at that revision. Original tables and artwork remain
private. Native projection/lighting limits are recorded in
[the encounter scope](docs/outer-wall-armor-lord.md).

Selected stone control in `modules/core/player_stone.c` follows
`src/dra/71830.c:PlayerStepStoned` and selected `PlayerStepKill`,
`src/dra/5D5BC.c:HandleDamage` / `GetStatusAilmentTimer`, and timer semantics in
`src/dra/6DF70.c`, at the pinned reconstructed revision (AGPL-3.0-or-later).
The selected palette/pivot renderer uses private common art; rotation tables
remain private. [Petrification scope](docs/player-petrification.md) records the
remaining statue effects, lethal variants and transform parity gaps.

`modules/core/enemy_medusa.c` adapts `src/st/e_medusa_head.h` at reconstructed
revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Private overlay tables supply spawn profiles, entry motion and animation;
[the feature scope](docs/outer-wall-medusa-heads.md) separates component fixtures
from route and console parity evidence.

BO4 resource/common-table adapters follow the stage's `header.c`, `e_init.c`,
`e_breakable.c`, `e_red_door_tiles.c`, `e_lock_camera.c`, shared
`st/entity_lock_camera.h` / `e_collect.h` and `dra/lba_stage.c`, at the pinned
reconstructed revision (AGPL-3.0-or-later). Only addresses, dimensions, filenames
and checksums are public metadata. [Arena scope](docs/doppleganger-arena.md)
records the disabled travel gate and unintegrated boss runtime.

The BO4 packed-sheet path in `modules/render/actor_graphics.c` adapts the sheet
branch in `src/dra/4B758.c` and the boss header/spritesheet metadata at the pinned
reconstructed revision (AGPL-3.0-or-later). Pixels and tables remain private.
[Sprite scope](docs/doppleganger-sprites.md) records unbacked skipped definitions
and the remaining modified-VRAM/transform/GPU parity work.

The BO4 timeline decoder adapts `src/boss/dop_anim.h` at the same pinned
sotn-decomp revision under AGPL-3.0-or-later. Original frame/property/pointer tables
remain private runtime data. The bounded adjacent-property read preserves an
original image-byte alias without native undefined behavior.

BO4 collision and thinking adapt `src/boss/dop_collision.h`,
`dop_check_st_collision.h` and `bo4/doppleganger.c:DopplegangerThinking` at the same
pinned AGPL-3.0-or-later revision. [Runtime scope](docs/doppleganger-runtime.md)
records explicit integer/ownership changes and remaining encounter work. Original
sensor tables remain private; public tests use synthetic geometry.

BO4 normal body controls adapt `bo4/dop_anim.c`, normal-form steps in
`unk_465F0.c`, `unk_46040.c` and transition/attack helpers in `unk_45648.c` under
the same pinned AGPL-3.0-or-later reference. Runtime transition tables remain
private; bounded request queues replace direct global entity/audio calls.

BO4 child factories and hand-hitbox decoding adapt `bo4/unk_49A6C.c`,
`bo4/unk_50D4C.c:DopplegangerUnarmedAttack` and `boss/dop_anim.h:UpdateUnarmedAnim`
at the pinned AGPL-3.0-or-later revision. Blueprints, allocation ranges, weapon-pose
and hitbox tables stay private. Explicit generations, bounded namespace/ranges and
primitive cleanup replace global pointer ownership; see the documented component scope.

The BO4 knife child adapts `bo4/unk_50D4C.c:EntitySubwpnKnife` at the same pinned
AGPL-3.0-or-later reference. Its native two-color line presentation uses checked
integer interpolation and existing RGB555 blends; `dra/4DA70.c` supplies original
primitive-command context. Runtime artwork/palettes and attack profiles stay private;
GPU endpoint/ordering parity remains unproven.

BO4 bat/mist body controls and white-blink geometry adapt `bo4/unk_465F0.c`,
`unk_49A6C.c:EntityDopplegangerBlinkWhite` and selected
`dra/4B758.c:func_800EB758` logic under the pinned AGPL-3.0-or-later reference.
Runtime profiles, sprite offsets and artwork remain private. Bounded native display
cache and Gouraud presentation do not establish console pixel parity.
`modules/core/game_sqrt.c` adapts `SquareRoot12_impl` from PSY-Z
`b52003e55c0d1b20719e984dd19bd4d760eeb14a`,
`psyz/src/psyz/libgte_sw_sqrt.c`, under MPL-2.0; the
[full notice](licenses/MPL-2.0.txt) ships with releases. Its lookup remains private.

BO4 hit/physical-death controls adapt `bo4/unk_465F0.c` damage, reset and kill
logic plus the selected timer/status order in `bo4/doppleganger.c` under the pinned
AGPL-3.0-or-later reference. The native erasure bitset represents the source's
nibble-clear algorithm; artwork, sound/effect selectors and scratch-rectangle
metadata remain private. Render-cadence comparison covers the component fixture,
not a console replay or complete elemental effects.

BO4 root orchestration, smoke, wing trail/hitbox and profile-driven impact/death
particles adapt `bo4/doppleganger.c`, `unk_49A6C.c` and selected `boss/dop_anim.h`
logic at the pinned AGPL revision. Stage collision selector metadata stays private
(`st/collision.h`). Contiguous allocation follows `dra/4DA70.c:func_800EDB58`;
BO4's initial graphics bank follows `dra/game_handlers.c:Play_Init`. Counts/pool
bounds, checked ownership and initialized native records replace global memory.
See the explicit [integrated fixture limits](docs/doppleganger-runtime.md#stage-combat-orchestration).

Library Spellbook/Magic Tome behavior, page debris, owned beam and face assembly in
`modules/core/library_book*.c` adapt `src/st/lib/e_spellbook_magic_tome.c` at the pinned
sotn-decomp revision above (AGPL-3.0-or-later). Original vertex/UV/recipe tables,
initialization, animation, attributes and artwork remain private runtime inputs.

Selected Lesser Demon spit, bolt and line-trail behavior in
`modules/core/library_demon_{spit,bolt}.c` adapts
`src/st/lib/e_lesser_demon.c:{EntityLesserDemonSpit,func_us_801BCC10,func_us_801BC814}`
at the pinned sotn-decomp revision (AGPL-3.0-or-later). Private animation, sensors,
SDK trigonometry and artwork remain runtime inputs. The original US initial
uninitialized-pointer vertex read is replaced explicitly; read
[attack scope](docs/library-demon-attacks.md) for this memory-safety adaptation.

`modules/core/library_mudman.c` adapts `src/st/lib/e_mudman.c` and selected
`src/st/e_misc.h:MakeEntityFromId`; `library_summon.c` adapts
`src/st/lib/e_lesser_demon.c:func_us_801BB8DC`. Library Skeleton changes follow
`src/st/e_skeleton.h`, all from the pinned sotn-decomp revision (AGPL-3.0-or-later).
Private appearance, motion, sensor, hitbox and artwork tables remain runtime inputs.

`modules/core/library_demon.c`, `library_demon_motion.c`, `library_demon_effects.c`
and `library_demon_summoning.c` adapt selected body, punch, windup and summoning
functions from `src/st/lib/e_lesser_demon.c`, with selected `st/prim_helpers.h`
and `st/e_misc.h` algorithms at the pinned sotn-decomp revision (AGPL-3.0-or-later).
Private tables remain runtime inputs. Paired effect projection in `boss_projection.c`
uses selected PSY-Z SDK matrix/column-scale/saturation algorithms at the pinned
MPL-2.0 revision; its original trigonometry stays in the private executable.

`modules/core/library_event.c` adapts selected `src/st/lib/unk_3B200.c` and
`unk_3B53C.c` algorithms from the pinned sotn-decomp revision (AGPL-3.0-or-later).
Library camera unlock follows `src/st/lib/e_lock_camera.c`; shared reward conversion
follows `src/st/e_life_up.h`. Tile/drop tables remain private runtime inputs. Native
seal signaling checks owner generations rather than writing beyond its child block.

`modules/audio/sequence.c` and `sound_sequence.c` adapt selected reconstructed
`src/main/psxsdk/libsnd/seqinit.c`, `seqread.c` and `vmanager.c` algorithms at the
pinned sotn-decomp revision (MIT). The [SDK/decomp notice](licenses/PSY-Z-decomp-MIT.txt)
retains the reconstruction contributors' attribution. Original sequence bytes,
instruments, samples and pitch entries remain private runtime inputs. Selected
playback selection follows `src/dra/sound.c` (AGPL-3.0-or-later); the native adapter
retains separate XA and SEQ ownership.

Selected Librarian controls/chair in modules/core/library_librarian.c and Library
script paths in entrance_death.c adapt st/lib/e_shop.c, cutscene.c and shared
cutscene helpers at the pinned sotn-decomp revision (AGPL-3.0-or-later). Scripted
player hold follows dra/692E8.c, 71830.c, 6DF70.c, 6E42C.c and alu_anim.c.
Dialogue, animation, portraits, samples and original tables remain private inputs.

Selected Library catalogue/transaction rules in `modules/core/shop.c` adapt
`st/lib/e_shop.c` at the pinned sotn-decomp revision (AGPL-3.0-or-later).
Equipment ownership/order and spell learning follow `dra/5D5BC.c`; purchased-map
reveal follows `dra/5087C.c:func_800F2288`. Original prices, stock rows, unlock
mappings, text and map rectangles remain private runtime inputs.

Selected menu controls/geometry in `modules/core/library_shop.c` and
`library_shop_graphics.c` adapt the pinned `st/lib/e_shop.c`; input repeats follow
`dra/pads.c` (AGPL-3.0-or-later). UI text/artwork remain private. Native draw priority
order now follows `dra/42398.c`'s forward `ClearOTag` / `DrawOTag` path rather than
assuming the reversed ordering-table direction. Full GPU packet parity remains pending.

Selected Library candle-table behavior and decoration initialization adapt
`st/lib/e_candle_table.c`, `background_block_init.c`, `st/e_room_bg.h` and
`st/e_room_fg.h` at the pinned sotn-decomp revision (AGPL-3.0-or-later).
Animation, object-init, equipment and permanent-item tables stay private.

Selected Library parallax behavior in `modules/core/library_background.c` adapts
`st/lib/e_library_bg.c` at the pinned sotn-decomp revision (AGPL-3.0-or-later).
Its bounded axial projector reuses the existing MPL-2.0 PSY-Z divider/saturation
backend. Model, anchor, lighting, texture and palette records remain private.

Selected rotating Library bookcase behavior/subdivision in `library_shelf*.c`
adapts `st/lib/first_c_file.c:func_us_801AE8E8` and
`st/e_misc.h:UnkRecursivePrimFunc2` at the pinned sotn-decomp revision
(AGPL-3.0-or-later). Selected projection/lighting in `boss_projection.c` follows
the pinned PSY-Z fixed GTE implementation (MPL-2.0), with original NCDS command
fields and MIT SDK `src/main/psxsdk/libgte/fog_01.c` fog arithmetic. All model,
normal, matrix, pose and lookup tables remain private runtime inputs. See
[scope and reference corrections](docs/library-secret-bookcase.md).
