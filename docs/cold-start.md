# Original cold-start flow

The requested cold start means launching a fresh process into the original startup,
title and New Game/Continue flow, then reaching playable Alucard through its real
initialization. This is an additional project acceptance gate alongside the six
[Alucard workstreams](alucard-status.md). It remains open.

`--cold-start --game-data DIR` now opens the original title before creating any
player or gameplay stage. Press Start, then select an existing ordinary native save
in the attached filesystem Continue adapter. Empty/corrupt slots report an error;
they never start a checkpoint. Cancel returns to the original title. New Game,
name entry, Richter's prologue and the original SEL memory-card interface remain
unavailable. This is a selected cold Continue path, not completed New Game startup.

With no gameplay option the app still shows the bootstrap. `--play` retains the
development Continue/checkpoint shell and eagerly constructed Laboratory session.
`--entrance` and the existing local playtest snapshot start inside the castle with
documented development stats and opening equipment.

## Selected startup and cold Continue ownership

The native startup owner loads verified SEL, both title planes, the resident DRA
and executable before creating its own display and SPU. The resource catalog also
records the actual prologue planes and SEL graphics/audio without enabling their
unimplemented owners. Title uploads consume exactly32 transfers per0x40000 plane,
the original private column table and SIM13 palette. No missing bytes are padded.
The explicit native logo omission follows InitSel's `VERSION_PC` branch; it does
not implement the US MDEC logo/movie. Native file preparation replaces disc/BIOS
loading, and the existing native libc adapter starts at1 rather than claiming the
console BIOS's initial random state.

The original retail HandleTitleScreen phases own17 actual SPRT leases in SEL's
1024-entry primitive namespace, RGB fade, engine-clock Press Start visibility,
ordered two-lightning updates, shared libc cursor, sound63D and one final menu
request. Actual private textures render at512x240 through a4:3 native viewport;
CLUT indices resolve through the original palette grid. Exiting retires all17
rows before the menu receives control. Repeated draws advance no owner.

The SPU loads the four original resident banks in0/1/2/3 order and configures the
actual private Studio B reverb profile. This establishes those banks and the
selected sound processor, not all startup SEQ/CD/movie or SDK timing behavior.
The attached Continue adapter uses the existing native font, launch-slot controller
and ordinary v9 save store. Its filesystem labels and256-wide display are an
explicit native adaptation, not the original384-wide SEL menu. It reads all15
slots, retaining invalid files and never writing a save during selection or load.

Only valid selection prepares a real saved session. The candidate borrows the
published save-store owner until successful preparation; failed resource/session
creation leaves the startup display, SPU, save store and clock available for retry.
Successful preparation copies the actual SPU and replaces stage/player banks1/3,
retaining common banks, scripts, voices, ring, reverb and DSP phase. Preparation
queues bootstrap sound requests without rendering another sample frame. Engine
clock and held physical input carry into gameplay; held confirmation cannot become
a fresh inventory/attack/jump edge. The store transfers once, and stale focus events
from the departing title window cannot pause the new session. Ordinary save bytes,
resource grants and v9 layout are unchanged. Game-over return still uses the existing
native Continue shell; original return-to-title remains unfinished.

## Selected evidence

Public authored tests cover title phases/lightning/fades/request and failure
boundaries, title texture geometry/clipping/UV/color/read atomicity, resident bank
ordering/bounds, stage-bank preparation without a sound update, physical input
handoff, resource inventory and CLI mode exclusion. These use authored data.

Private original-instruction comparisons cover64 supplied retail contexts and
48,032 complete17-SPRT callbacks. Debug and ASan traces match byte-for-byte, including
color/lightning/random-control/Start/fade/menu requests; allocation, libc results,
display/clear/free/CLUT reset/game-state and sound calls are explicit adapters.
Native protocols separately check actual private uploads/SPU/GL, row generations,
held Start, repeated draws, real exit and isolated save selection, corrupt/empty
slots, late resource failure/retry, carried clock/input/SPU and unchanged save bytes.
These are selected native/controller/failure fixtures, not ordinary acquisition,
New Game/prologue, full menu/audio/whole-pass atomicity or console evidence.

All227 public groups pass in Debug, Release and macOS ASan/UBSan. Eight sequential
native protocols cover title, Continue/failure, empty/cancel/focus and actual app
initialization in Debug and ASan, each with an explicit pass and normal exit.
Four further native regressions repeat Magic Missile menu/freeze/restore continuity
and eleven-profile CD bootstrap/save-room/bank-rebinding checks in both builds.
Python34, repository policy996, changed C/header formatting, static SDL linkage and
Release archive CRC/private-input checks pass. Private traces and supplied native
contexts retain the evidence boundaries above; these checks do not close the gate.

## Source-backed implementation slices

1. **Startup and resident resources:** create the native boot owner before any
   development player/stage. Bind verified SEL, title/logo/menu/prologue graphics
   and audio through the existing private-resource boundary. Preserve initialization,
   cleanup and actual load-success gates; failed preparation must preserve the
   published owner and report the failed resource. Do not infer readiness from an
   invented delay. Original disc/BIOS loading needs a documented native adaptation.
2. **Title and menu:** reconstruct InitSel/HandleTitleScreen and the main-menu
   dispatcher with actual artwork,512-wide title presentation, original primitive
   order, fade/input/RNG and sound clocks. Press Start must enter the real menu.
   Continue must use the existing ordinary save store through an explicit filesystem
   adaptation, retaining empty/corrupt/load-failure handling and user saves.
3. **New Game initialization:** connect original name entry, game-clear/character
   branches, actual new-game requests and settings. Initialize original resources
   and progression from their real producers. The existing development checkpoint
   must remain separately identified. Metadata alone must not enable an unsupported
   character or progression branch.
4. **Richter prologue and result:** bind actual RIC/ST0 player/stage/boss/cutscene/
   audio/movie owners and preserve skip/retry/rescue/result conditions. Carry the
   real outcome through HandlePrologueEnd and original Alucard starting bonuses.
   A supplied prologue-result fixture can verify that component but cannot establish
   playable prologue or ordinary New Game availability.
5. **Alucard arrival and return paths:** connect the original forest/Entrance opening,
   player initialization, Death and normal connected play. Verify a fresh process,
   ordinary save, quit and cold Continue without development grants. Preserve real
   game-over/title return and resource retirement. Only then select this startup
   from ordinary app launch and the local playtest wrapper.

## Acceptance

Require actual native startup from a fresh process, keyboard/Xbox Start and menu
navigation, New Game through the real prologue/Alucard handoff, ordinary saved
Continue from another fresh process, and game-over/title return. Repeated draws,
loads, menus and failures must not add simulation/RNG ticks or duplicate audio.
Exercise invalid/missing private resources and failed candidate/session creation;
retain existing saves and published owners. Record original-instruction comparisons,
public authored tests, native runs and remaining platform concessions separately.
Original artwork/resources remain private; packages and CI use no game bytes.

Current component and native checkpoint checks do not close this gate. Full castle
routes/endings, console parity and the six Alucard workstreams remain separate.

Reference inventory: sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, `main/main.c` (resident loader),
`dra/42398.c:MainGame` (global initialization), `dra/game_handlers.c:HandleTitle`,
`MainMenuHandler`, `HandleVideoPlayback`, `HandlePrologueEnd` and `HandleNowLoading`;
`st/sel/34048.c:InitSel`, `3410C.c:HandleTitleScreen`, `2C048.c` (main menu/name
entry), `st/st0` and `ric` (prologue owners), `dra/5F60C.c` (Alucard initialization/
prologue bonuses), and the Entrance opening owners. These references establish
implementation boundaries; they do not establish current native availability.
