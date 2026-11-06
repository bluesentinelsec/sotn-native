# Selected Monster vial 2

Monster vial 2 (item2) is a kind130 consumable running the weapon41 program (`w_041`) in either
hand. Each press throws a vial that breaks in the air into a bat, which seeks the nearest enemy
ahead. Defaults, grants and saves are unchanged; the stock is ordinary inventory.

## Use

Kind130 takes the existing throw path:

- CheckChainLimit (chain eight; the live vial carries item2);
- the blueprint50 factory (E0/F0 in slots17..31, with the hand bit);
- the casting grunt and animation 0x60;
- one consumed vial after the player's successful update (Duplicator keeps it).

## Vial

`EntityWeaponAttack` (role0) uses the same sprite setup as Monster vial 1 (bank 0x8010/0x8012,
palette 0x110/0x128, unk5A 0x64/0x66, zPriority+2, CAMERA_LOCKED|0x100000) with
`D_123000_8017A504`, four pixels up. The program cursor `D_123000_8017B204` (reduced modulo four)
picks one of four `D_123000_8017A53C` speeds, then advances. Modulo three, the advanced cursor
picks one of three `D_123000_8017A54C` rises. It also sets a (rand()&15)+24 lifetime and
`g_Player.timers[10]` = 4.

It falls with 0.15625 gravity and checks no collision. When its lifetime ends it breaks in the
air: `D_123000_8017A4DC`, BLEND_TRANSP|ADD, `PlaySfxVolPan(GLASS_BREAK_A, 0x50, 0)` and
`WFACTORY(56,0)`. It retires at the animation's END.

## Bat

`func_ptr_80170004` (role1, slots17..31) copies its raw parent vial's animSet, unk5A, palette and
zPriority-2. It turns to face the opposite way and grows by four per update to 0x100. There it
calls `SetWeaponProperties` with the raw parent's **current** equip id: usually the empty hand's,
since the vial has retired (as in [Monster vial 1](player-monster-vial-1.md)). Its group is its
own least-used one.

Eight updates later it calls `func_123000_8017A994` with heading 0 (when facing left; 0x800
otherwise) and tolerance 0x280, then plays BAT_WING_SWISHES and BAT_SCREECH. The selection
scans stage slots64..191, keeping candidates that:

- have an entity and a hitbox, and lack flag0x200000;
- are on screen with HP below 0x7000;
- lie within the window (ratan2 from the bat; differences above 0x800 wrap).

The first unmarked candidate is marked (FLAG_UNK_80000, shared with the spirits) and chosen.
Otherwise the program's own circular cursor `D_123000_8017B200` takes the next marked candidate
in the window. The runtime performs this step with the bat's screen position and commits a new
mark with the existing spirit-mark ownership.

Each later update:

1. `func_80118B18` gives the heading to the retained target slot, or to the default edge point
   when there is none.
2. The bat turns toward it by at most its turn limit. The limit starts at 0x80 and only
   shrinks: whenever the remaining difference is smaller, it becomes that difference.
3. It flies at `rcos/rsin(heading) × 24` (1.5 pixels per update).

Nothing reaches its original step4, so the bat flies until offscreen culling retires it.

Every bat callback ends with DRA `UpdateAnim` over `D_123000_8017A50C`'s frame-property hitboxes.
It also writes colour8 of the hand's first weapon palette (0x039C or 0x199D, alternating every two
game updates); the session commits the write after the pass.

## Graphics

The program's palette table (0x8017A558) uploads twenty-four palettes, with the hand word at
0x8017A55C. Sprites use the standard weapon bank table.

## Evidence

Public tests cover:

- The angle-windowed selection: the window and its wrap, non-candidates, marking, the cursor walk
  and a negative cursor.
- The vial's cursors, lifetime, flight, break and END retirement.
- The bat's parent words, flipped facing, growth, properties, wait, selection request and sounds;
  turn-limited homing on a bound target and on the default point; velocities, hitboxes and the
  palette flash.
- Shared-runtime throws: an unmarked target gains a committed mark, and an already-marked one is
  found through the program cursor without a new mark.
- The player's consumption.

A private MIPS oracle executes both original callbacks over 4,561 traced callbacks, running:

- the original selection over 128 emulated stage entities, with real ratan2, its marks and its
  cursor;
- real DRA `func_80118B18` homing, real `UpdateAnim` and the palette flash.

The trace replays the runtime's selection step. Debug and ASan traces are byte-identical.

A private native LLDB probe in Laboratory room 3 throws real vials from the equipment menu in
both hands, by actual pad input. The bat picks the enemy placed ahead, homes in and damages it,
and one vial is consumed. These fixtures supply items, stock, protection and positions; they are
not acquisition, route or console evidence.

Adapted from `src/weapon/w_041.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under
AGPL-3.0-or-later. All original definitions, tables, palettes and sounds remain private.
