# Original cold-start flow

The requested cold start means launching a fresh process into the original startup,
title and New Game/Continue flow, then reaching playable Alucard through its real
initialization. This is an additional project acceptance gate alongside the six
[Alucard workstreams](alucard-status.md). It remains open.

The current executable does not provide that flow. With no gameplay option it
shows the bootstrap; `--play` eagerly constructs a development Laboratory session
and opens the native Continue/checkpoint shell. `--entrance` and the current local
playtest launcher start inside the castle with documented development stats and
opening equipment. These are useful checkpoints, not completed New Game startup.

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
