# Selected Icebrand

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts Icebrand113 through
the weapon50 program, with its QCF special181 (kind22). Like the other brands' specials
([Thunderbrand](player-thunderbrand.md), [Firebrand/Marsil](player-firebrand-marsil.md)), it
holds pose0 for forty updates, costs five MP after the factory and plays no casting grunt. The
program's palette table and live hand word (D_8017B068/D_8017B07C) load through the existing
palette owner. Defaults, grants and v9 saves are unchanged.

## Attack callback

The four descriptors draw additively from the program's bank0x10. Descriptor1 (the special)
plays SFX_MAGIC_WEAPON_APPEAR_A at birth and requests the WFACTORY(0x3A,0) sparkles and the
WFACTORY(0x40,0) ring before SetWeaponProperties. On the first update with pose1 after pose0
(prev_player_pose), descriptor1 requests the WFACTORY(0x44,0) frost streaks with
SFX_FM_EXPLODE_B and SFX_MAGIC_SWITCH. Then every descriptor requests FACTORY(0x61,descriptor)
crystals, so even Icebrand's ordinary swing leaves crystals.

## Children

- **Sparkles** (E1/F1, slots32..63). Eighty mandatory GT4 rows (palette0x1B0, page0x1A, hidden
  transparent drawMode0x339) wait 1..24 updates. Each then picks a random angle and radius
  32..63 around the blade (±12, -26, or -31 when Y-scaled) and stores the point in the
  program's D_8017CBFC. It drifts there over fifteen steps through func_8017B784's six UV rows,
  two placements each (half-extent six, then four), until row6 hides it. The child retires
  after forty updates or outside animations0x41..0x47. Native code keeps each row as centre,
  UVs and the original scratch bytes inside the existing child storage and rebuilds the GT4 at
  draw time; the lease uses all eighty actual rows.
- **Ring** (E2/F2). Sixteen LINE_G2 rows retyped as white-fill GT4 wedges grow from radius4 by
  six to forty. Unlike Firebrand's ring, it copies the raw parent's equip id, rolls properties,
  copies the enemyId and attacks with a box of radius minus four. All three channels fade over
  its last sixteen of forty updates.
- **Frost streaks** (E3/F3, slots17..31). Thirty-two mandatory LINE_G2 rows, each with six libc
  calls (colours, ±12 offsets, a 1..5 delay), lie along the blade anchor's three-quarter extent.
  Each shows for exactly one update after its delay. The child copies the raw parent, attacks
  with a 24x8 box at X+24 and retires after eight updates. It re-anchors to the player
  animation every update, even outside the attack window, reading the program's words beyond
  D_8017B030 as the original does.
- **Crystals** (E4/F4). Eight GT4 rows along the blade (three quarters of the extent for
  descriptor1, half otherwise, plus rand%10/rand%4) write their points into the same
  D_8017CBFC words as the sparkles. Each rises one pixel per placement after a 2i+1 delay and
  retires after 0x18 updates or outside the attack window. Descriptor0 adds DRAW_UNK_40. The
  original scratch bytes stay in the rows' colour fields, which this drawMode ignores.

## Evidence

Public tests cover the attack's sounds and factories; the sparkles' delays, drift schedule,
sizes and retirement; the ring's properties, box and fade; the frost streaks' RNG order,
one-update flashes and lifetime; the crystals' shared points and rise; the runtime's 80/16/32/8
row leases and release; and Icebrand's kind22 QCF. Mutations of each are caught.

The private original-instruction comparison runs complete w_050 callbacks: 31,136 attack
callbacks and 2,776 child callbacks across all four roles. That covers 67,584 primitives,
including 12,006 shown sparkle placements, the shared points and out-of-window frost anchors.
Its 34 full palette copies match the compiled loader. Debug and ASan traces are byte-identical.
Native Debug/ASan protocols equip Icebrand through the menu in either hand and run its ordinary
swing (crystals) and the real QCF special. Every child's pixels show, secondary damage lands
outside the blade's box, and rows are released. Supplied stock, protection and position
contexts are not acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_050.c`, `weapon/shared.h`, `dra/6E42C.c` and
`dra/71830.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
