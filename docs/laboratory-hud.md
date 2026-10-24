# Original Laboratory HUD

The checkpoint presents the original Alucard HUD over its 256x240 frame, using
layout tables and artwork from the user’s verified DRA and F_GAME files. It shows
current HP, the MP gauge, hearts, and the collected subweapon. No replacement
font, recreated artwork, or generated game arrays ship with the app.

HP follows the original counter interpolation, including full-health and
low-health palette selection, digit spacing, and leading-zero suppression.
The rotated MP strip uses original pixels and its full-gauge palette cycle.
Subweapon artwork, frame color cycling, glints, and available-heart flicker
read the original runtime definitions. HUD state advances on simulation ticks;
extra draws and focus pauses do not advance it.

The development profile still has 70 maximum HP, base stats 6, 20/20 MP,
10/50 starting hearts, no armor/accessories, and Cube of Zoe enabled. There are
no MP-consuming abilities in the checkpoint, so the game currently supplies a
full 20/20 gauge. That is explicit development state, not recovered progression.
Subweapon use and the inventory menu remain incomplete. Owning a subweapon
can enable the original HUD availability flicker even though its throw action
is not integrated. Global RNG consumption, the complete shared palette engine,
all HUD effects, and whole-game audiovisual parity remain unproven.

`modules/render/hud.c` adapts selected `src/dra/5F60C.c:DrawHud`,
`DrawHudSubweapon`, and `4A538.c` palette paths from the pinned reconstructed
revision under AGPL-3.0-or-later. Metadata is parsed with explicit byte order
and bounds; runtime pointers are never treated as host pointers.

Synthetic tests cover numeric artwork selection, full/normal/low HP colors,
interpolation, leading digits, full/partial MP fill, subweapon animation,
render-independent timing, and malformed status/layout/palette references.
The native LLDB combat encounter passed with the HUD enabled, including player
damage and a heart pickup. Private captures remain ignored. These checks do not
prove complete original timing or listening/visual parity.
