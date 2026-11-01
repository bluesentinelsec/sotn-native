# Selected queued contact numbers

Actual queued hurt, lethal and stone contacts now create original type0 numbers
through the shared entity19 owner. Surviving hurt and selected lethal contacts
request red numbers. New and existing stone also request a number, including a
blue zero when stone causes no HP loss. Immunity and guard retain their separate
existing type0 producers; absorption and deferred healing retain green type1.
Talisman and ordinary water pain/death create no additional contact number.

The request retains original damage points before the HP cap and pre-reaction
fractional origin. It does not substitute HP actually lost: a 1,000-point lethal
request still shows1000 against lower HP. Weakness and existing poison can yield
39996 requested points; the selected reaction helper's9999 HP-loss cap does not
truncate that number. Stone retains its own zero/minimum and maximum-HP-fraction
rules. The renderer uses the original unsigned-word interpretation, red/blue
palette choice and four least-significant digits, including amounts above32767.

Only accepted queued contacts produce a request. A helper refusing damage, an
invincible/dead/grabbed player or failed player update cannot create another
number. Existing stone can finish escape during the same frame; its accepted
response8 number remains valid after that state change. Pending healing still
commits first and creates its independent earlier green number. Blood Cloak,
Fury, ailments, reaction RNG, HP and ordinary resource ownership retain their
existing boundaries. This adds no reaction kinds, elemental death, item grants,
RNG stream, timer, resource pool or save fields.

## Actual ownership and timing

After successful player commit, the shared runtime allocates a direct entity19
in the first free slot56..63. Each number leases twenty actual stage rows: four
private digit GT4s and sixteen trail tiles. Allocation failure omits only the
optional visual; the number consumes no RNG. It never
replaces unrelated children. Its motionless birth and original twelve phases,
SDK math, red/blue trails, rendering and retirement share the existing
[number owner](player-healing-numbers.md); no parallel implementation is added.

One-update producer feedback clears on the next ordinary/frozen/release player
update. Initialized numbers continue through actual frozen effect updates while
HP/player timers hold; release-only frames, menus, map and repeated draws hold
their motion. Room/camera rebases translate retained points/packets without a new
update or replay. Full stage/Continue replacement clears transient entities, as
it does for the other shared number producers. Game-over presentation holds the
remaining scene until its existing owner is replaced. Saves remain v9 and no
transient request or primitive lease is serialized.

## Evidence and remaining work

Public authored regressions cover hurt, overkill through39996, zero/new/lethal/
existing stone, same-hit stone escape, immune/absorb/guard/Talisman/water exclusions,
prior healing and ordered green/red coexistence, fractional capture/rebase,
failed-player rollback, actual shared leases, frozen motion, optional exhaustion,
invalid requests and row retirement. No original assets/tables are embedded.

Private instruction comparisons match201600 original full HandleDamage/number
response dispatches:126552 selected requests,21744 zero-stone cases and33624
requests larger than the preceding HP. They compare requested word/type/full
fractional origin, HP/response and existing Cloak/Fury work. Recalculation/HUD,
equipment, AddHearts, factory, number and audio services use explicit adapters.
The existing type0 controller/private-SDK/packet comparisons retain their separate
scope; these comparisons do not establish complete dying-player frames.

Muted native checks use default70HP/20MP Laboratory starts and actual zombie
contacts: a real hurt number contributes21 pixels, and seven unmodified-health
contacts include a lethal number through the actual game-over transition. Checks
also exercise six menu holds, actual spirit freeze33/release1, real twenty-row
leases and retirement. A separate owned-Mask28/raw-stone fixture produces a blue
zero contributing18 pixels without HP loss. The actual CMake app checks ordinary
hurt, then controlled zero-stone/tapped escape and a lethal1000-point request.
Item, raw-contact and later protection fixtures are not acquisition, ordinary
Medusa/lethal encounters, uninterrupted routes or console evidence. No HP/MP/RNG/
clock grants or user-save writes establish these checks. Final platform results
are recorded in the feature PR.

Full hurt promotion/kinds, elemental/interrupted death, stone variants, secondary
impact/grab/MP number producers, other equipment/familiars and shared contention
remain unfinished. See the [damage inventory](player-contact-absorption.md).
Milestone1 remains26/34 handlers plus common/cross-owner work; Alucard and castle
remain incomplete.

Provenance: `dra/5D5BC.c:HandleDamage`, `692E8.c` response3/4/7/8 dispatch,
`7879C.c:CreateHPNumMove`/`EntityNumberMovesToHpMeter`, pinned AGPL-3.0-or-later
`sotn-decomp` `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. Data/artwork stay private.
