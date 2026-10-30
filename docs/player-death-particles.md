# Original death particles and initialization

Selected entity-five profile6 now uses the shared factory, primitive and drawing
owners. Its motion comes from the original32-entry velocity tables generated at
full player initialization, rather than a substitute particle pattern. This
prepares another dependency of water death; the dying-player producer, capture
and remaining common death operations are still unwired.

Initialization consumes64 values from the selected shared character RNG and uses
the original SDK sine/cosine table, signed shifts and intensity ordering. The
native session initializes these tables once after new/continued character
restoration. Room entry, particles, menus and drawing retain them without another
initialization charge. A fresh full-stage session or Continue generates new tables
from its actual restored RNG. Initialization fails atomically on invalid inputs.
The project still has other separately owned RNG streams; this is not a claim
that all original global randomness or initialization ordering is integrated.

Each actual profile6 child reserves its private count plus one sentinel row before
consuming randomness. Birth uses three RNG values per particle for wrapped whole
positions and a retained vector index, without facing substitution or copied
source fractions. The single lifetime decrement, signed damping, additional
eighth-update halving, draw-mode change and strict thickness thresholds preserve
the original behavior. Colors do not acquire a generic fade or gravity. Optional
row exhaustion retires before RNG; missing initialized vectors fail explicitly.
Rebases retain fractions and do not advance lifetime. Existing unrelated primitive
owners and factory/child identities remain protected.

Public authored tests cover initialization cost/rollback, real shared scheduling,
allocation failure before RNG, retained vectors, wrapped positions/fractions,
damping, mode/thickness/retirement boundaries and malformed lifetime rollback.
Private Debug and ASan/UBSan traces match byte for byte. Original instructions match
256 initialization/SDK loops,96 particle contexts and2,352 updates with27,072
particle records. Allocation, RNG and destruction calls use documented adapters;
these supplied contexts are not native dying-player or console evidence.

Muted native Step/Draw checks exercise actual factory49's56 births, private13-row
children,2,632 updates and31,584 tile draws, retirement and menu holding. They also
check room-entry vector/RNG retention and fresh in-memory Continue initialization
from restored RNG. They use explicit position, fixed-floor pose, contact protection
and a supplied factory request; they write no user save slots. These checks do not
establish ordinary deaths, uninterrupted travel, complete global contention or
console parity. An actual CMake-built Debug app checks initialization and100
ordinary Entrance updates under muted LLDB, then exits through normal SDL quit,
without position/stat/relic/protection/factory fixtures.

This adds the original64 initialization RNG calls to full native session starts.
HP, MP, equipment/relic grants and version-nine save fields remain unchanged.
Original private vector inputs, profiles, artwork and traces stay excluded from
public source and packages. Water damage/death, full effects/equipment, Alucard,
the castle and console parity remain incomplete.

Provenance: selected `dra/692E8.c:AluInit` and
`dra/7E4BC.c:func_8011E4BC`, pinned `sotn-decomp`
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
