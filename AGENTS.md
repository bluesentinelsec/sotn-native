Selected [shield combos](docs/player-shield-combos.md) now resolve CheckWeaponCombo (D_8013AEE4)
on every equipment set and cast every shield spell (Leather203/Knight204/Iron205/AxeLord206/
Herald207/Dark208/Goddess209/Shaman210/Medusa211/Skull212/Fire213/Alucard214/Alucart215) with
Mablung or Shield rod on both buttons (held shield counts): combo hand, CheckChainLimit on the spell's live children, MP before, <<14
factory bits (plain factory, E0/F0 offset), MP after, kind5. The w_008 E8 spell (11 GT4, screen
rows) sets unk28=1 (freeze1 = spirits' freeze plus BlinkWhite0x47/0x40 flagged0x10000), grows,
flashes, opens a nine-band card on retained program shieldAngles and releases FACTORY(55,8);
Knight (w_009, 12 rows) slices ten horizontal RGB bands, 48x40 card, pink7, bright<4 fade;
Alucart (w_058, 2 rows) holds 0x40, flashes (0x1FC) and shows the page-0x1E question mark;
Axe Lord (w_011, 10 rows, 20 MP) resets its band angles, ends the freeze itself and creates the
blueprint88 E9/F9 walker (raw-parent copy, least-used group, no parent enemyId, params without
the hand bit) whose blueprint90 afterimages (32..63) keep equipId0; raw parents read only at birth.
Goddess (w_025, 17 rows, 40 MP) keeps fifteen program angles, requests FACTORY(90,1) E9/F9 clouds
(one row, 32..63, flag0x10000, six birth rands, raw parent read each aim; empty slot reads screen0)
and pink15; spell/clouds keep screen position. Lab GT4 commands pack owner*32+row.
Shaman (w_026, 14 rows) keeps program angles/green cycle/glass counter, rewrites palette0x112
colour13 green each callback (read-modify g_Clut, committed after the pass), releases
blueprint92's eight GTE stars (RotTransPers4, screen0x280; ChildRows: star1/trail4 rows) and
pink6; stars read the raw spell's step, trails copy the raw star's row.
Alucard (w_052, 23 rows incl. 11 G4 ribbons drawn untextured) arms its shield's program
(D_8017ABBC/ABC0=0x1000): the raised family52 shield attacks (hitbox|2, cached attack), holds MP
regeneration (D_80097BF8 -> HUD mp_hold), drains MP each 16th game update (native MP stops at 0),
heals/hearts/BlinkWhite0x15/timer13 on hits (SotnPlayer_CommitShieldHit) and at timer/MP expiry
rolls properties again and releases the 0x3A E1/F1 letters.
Iron (w_010, family10, item7 now a guarding shield): a raised hit requests WFACTORY(56,0)'s E1
thunder (raw slot16 must hold item7; palette0x8194, THUNDER_A, anim0xBA0) then guard_blink
(BlinkWhite0x15 + protection0x60 via SotnPlayer_CommitProtection, no heal). Spell205 owns 33 GT4
(spell_quads[33]; lab GT4 commands now pack owner*64+row), program iron_angles/angles2 (0xBBC/0xBDC),
FACTORY(88,0/1) E9 swords: 18 rows, FakePrim screen centre (row0 x0/y0), delay0x20 then
CAMERA_LOCKED (screen retention only before), equip 205, least-used group, culled offscreen.
Herald (w_023, family23, item9): a raised hitFlags3 block with unk84==0 calls AddHearts(1)
(shared helper: below max only, rising heart FACTORY(99,0) + HEART_PICKUP; Alucard shield's hit
uses it too) and sets the 0x20 cooldown, decremented every update. Spell207 (18 GT4, s32 program
angles 0xB70) requests FACTORY(94)'s 24 E10/F10 sprites (params0..23 wait params*4; params0
wraps to 65536, kept) and FACTORY(90)'s four-GT4 E9 swirl (RotTrans vectors in program BSS,
RotTransPers4 x0/x1/x3/x2) before pink9; pink now follows a spell's factories. All screen words.
Dark (w_024, family24, item10): rand()&3 palette sequence (0xCF8) before properties, stepped
every non-final update, rerolled at 0xFFFF. Spell208 (30 MP) sets g_Player.unk20 (transform
lockout, native mist_lockout via SotnPlayer_SetTransformLockout) every callback; rows0/1 are
subtractive TILEs; FACTORY(96) rocks + freeze end + own SetWeaponProperties 0x7F hitbox for three
callbacks at opening update32; rocks (E9, 1 GT4, 4 rands, least-used group, 0x40 hitbox) read the
raw spell's step each update (excluded from the generic parent_phase override).
Medusa (w_027, family27, item13: plain guard; Dark/Medusa hit state6 also attacks). Spell211 (one
glow row, spell timeline word 0xC8C/0xCB4) requests FACTORY(98)'s two E9 lasers (18 rows, own
laser_slot/laser_bound from func_80118970, TargetHeading = func_80118B18, SquareRoot0 swing), which
request FACTORY(100)'s E10 hit points (item0 properties, 6x6, six updates; sixth-hit release
unreachable). laser_quads overlays spell_quads. Oracle adapts SquareRoot0's COP2 LZC.
Skull (w_028, family28, item14, plain guard 0xAE0). Spell212 (13 rows, ten program angles 0xB1C)
runs the D_8017AB18/AB1A red ramp into palette0x113 colour11 each drawing callback, recoils with a
tremble, and requests FACTORY(88)'s E9 beam: three rows, raw parent X every update, and
SetWeaponProperties every phase-2 update with the raw parent's equipId re-read (zero once empty).
Fire (w_029, family29, item15, table 0xCC0): palette unk80+D_8017ACF8[g_GameTimer/2%5] every
update; non-flung shields request WFACTORY(0x38,0) every fifth game update: E1 afterimages (raw
slot16 item15 each update, CAMERA_LOCKED, OPACITY|MASK_R tint: red 128, twenty updates). Spell213
(1 row, 50 MP, flame timeline 0xD24) releases FACTORY(102)'s three E9 dragons (19 rows; their
D_8017C9A0 BSS segments live in fire_segments, screen words) and every update FACTORY(100)'s E10
hit points (14x14, ten updates). Every shield spell now has its owner.
Other combos keep the left-hand attack (gap) until their spells exist.

Selected [Holbein/Blue knuckles](docs/player-holbein-blue-knuckles.md) now own kind134 items136/137
through w_046: func_8010DFF0(1,0x20) (afterimage reset, timer15) then the kind129 throw path (chain1,
never consumed, animation0xD2). Holbein's blueprint50 E0 root (own item, one mandatory GT4, 15x20)
requests WFACTORY(0x32,1/2) E0 slashes (no item; params up to 2) whose corners spin around the LIVE
player; nine CLUT steps. Blue knuckles' blueprint56 E1 arc (32 mandatory untextured G4s) follows the
player, sweeps, fades and clears its hitbox below 0x80. Palette table 0x8017AC50, hand word 0xAC60.

Selected [Jewel/Stone swords](docs/player-jewel-stone-swords.md) now own items109/114 and QCF
special198 (five MP) through w_037 variants1..3. Variants1/2 request WFACTORY(0x3A,variant-1) on
the first pose1: E1 sparkles (slots32..63) roll the special's rand()&31 coin pattern BEFORE
sixteen mandatory GT4s, place eight sparkles from D_8017A714 (pattern/points/palette counter are
shared program state) and request WFACTORY(64,0) coins from marked ones. E2 coins (rowless,
parent slot up to 63, full factory table drops them) bounce on four CheckCollision probes, rest
0x50 and pay one gold (cap 999999, committed with the successful runtime tick). Stone has no extra
petrification effect: native combat has no element-specific enemy flash for any weapon.

The [Sword familiar component](docs/sword-familiar.md) (tt_004, card22, servant5, stats4) owns
ServantInit (48-word CLUT row0xF4), default follow, menu-switch entrance (speech 0x4FB..0x4FF),
the GTE model (own matrix, CompMatrix (0,0,6500), RTPS (128,128)/H1000, NCLIP, OTZ priorities),
blade trail (DA), level-90 halo (DB), events (table 0x801719E4, DC) and the DD sound cursor.
SotnLaboratory_SelectFamiliar(5)/engine init now load it through a per-familiar profile (card,
record, TT/FT/SD files, events): FT pages like every servant, only 48 CLUT words, no bank20 sprite.
One overlay is resident; switching familiars replaces it (fresh history, sound cursor 99). Real
stage-pool leases (model96/trail16/halo4, tag 0x9000|part<<8|slot) are checked before each
AllocPrimitives callback and committed after it; draws are GT4 rows at their own priorities. The
relic menu does not offer card22 yet. The component now runs func_us_80173AA0 (stage64..191,
mark 0x80000 on the first unmarked, else round-robin D_us_80178B88; no HP>0 check, raw slot
follow), the circle D2 (<70) and dive D3 (>=70) with libc rand, voices (D_us_80170090) and two
DE hitboxes per slash from slots17..46 (GetFreeEntity(0x11,0x2F), tips, spell19/20). The session
commits the marks (generation-checked), voices and DE children (SotnBatRuntime familiar_hitbox:
first update in slot order = Spell() INT/rand/Mojo * (level*4/95+1), least-used group, 4x4,
familiar_mask in combat; destroyed next update). Level 50 with castle flag464==0 adds item126
and sets 1 (read again the same update: D5 farewell, flag 2); D5 dismisses (card found-only,
g_Servant0, slots cleared, overlay kept). The relic menu switches card22 (other cards off, item126
unequip) and menu close loads/switches/same-reinits via LoadFamiliar. Item126 is an ordinary
weapon (see the Alucart block). Sword Brothers (spell7, step39 on the Summon handler, ripple
outline 0x170028, unk70 pulse on cast and combo completion) drives D4 (billboard, 64 copies,
ring) and its DF (spell7 global 0x82, zero size, nine updates). Square-root oracles: LZC 0x8001721C/0x80016EEC.

Ordinary [saves](docs/native-saves.md) are now v11: the seven familiar records (level/EXP/loads,
3636..3719, level = EXP/100+1) persist; v10 and older migrate to level-1 records. Continue
restores them before InitializeFamiliar. Older "v9/v10" notes describe v11's other fields.

Selected [Alucart set](docs/player-alucart-set.md): Alucart sword168 runs w_043 descriptor1 (no
special), Alucart mail89 is plain armor; IsAlucart (sword+shield167 in hands, mail in armor) adds
LCK30 before the cap and renames the inventory header. CalcAttack's Shield rod4 +5 beside a
category9 shield now applies to stats and each swing: SotnCharacter_HandProperties takes the hand.
Sun/Moonstone and Vlad-relic stat bonuses remain unimplemented. The Sword Familiar126 runs w_043
descriptor2 (green tiles, palette1 = g_WeaponCluts[1]) with ATK + record4 level (CalcAttack and
swing properties; RestoreFamiliars recalculates); equipping it turns the Sword Card off.

Selected [Shield rod](docs/player-shield-rod.md) now owns item4 (kind2, weapon7 descriptors 0xC28,
additive) and back-forward special173: descriptor1 requests WFACTORY(0x4A, anim) on pose4 at player
poseTimer7. The E1 orb (slots28..31) reads its raw slot16 parent only at birth, keeps equip id0,
takes fixed hit-type4 attack1 words and func_80118894's group, grows/decelerates, bobs for 0x200
updates shedding WFACTORY(0x40,0) sparkles every eighth, shrinks on countdown or hit (a hit on the
growth's last update strands it in the original's unhandled step4). Sparkles lease one optional GT4
before two rands; quads use the retained (pre-scroll) camera. Native combat now writes attacker
hitFlags for hit-state-4 guards too (collision.h does so on overlap); guards only meet bit4 hitboxes.

Selected [Sword of Dawn](docs/player-sword-of-dawn.md) now owns two-handed item17 (kind6,
weapon30 swing: DRA bank10, TRANSP|QUARTER) and QCF special177 (kind5, bp56, chain8, five MP;
equipped 0x11 also refuses vram flag0x40). The circle (E1, 11 GT4) rolls one/six soldiers then
scans fifteen CheckCollision ground points and creates WFACTORY2(0x3E) soldiers (free-factory
limited, E2 spacing); shared D_8017A5F4 bands, tether, wave unk84. Soldiers (E2, 14 world GT4)
wait for their raw circle's wave, rise, walk/shoot/throw/bomb (real UpdateAnim/frame props),
blink out on hits (type1 three lives; attacker hitFlags now reach every secondary and slot16). Arrows/swords (E3) home via shared target marks and
func_80118B18; bombs (E4) burst or roll with FACTORY(0x24,1) sparks. Unscanned points read
zero natively (original uninitialized stack); soldiers lease rows at birth.

Selected [Shotel](docs/player-shotel.md) now owns item28 (kind12, w_002-template weapon34
swing, BLEND_TRANSP) and its QCF special176: a kind129 throw (StartThrow, blueprint56, chain1,
never consumed, no MP) whose E1/F1 boomerang flies out (4.5 plus player speed, 0x1C updates,
ROT(45) spin) and homes back (turn limit 0x200 +0x10). While any 176 lives with either hand's
params (SetSpecialChain/SetSpecialCross), case12 punches: Shotel's factory, then 42+hand, item0's
animation/kind/lock.

Selected [Yasutsuna](docs/player-yasutsuna.md) now owns two-handed item165 through weapon53 as a
kind129 throw (blueprint68, chain1, animation0x5D, never consumed; either button). Its E3/F3
role3 root in17..31 stays at its factory origin (no forward offset; crouch +11 else -5), takes one
mandatory GT4 and, only at parameter0, rolls properties (least-used increment, then group0 from
the raw player parent) and requests WFACTORY(0x38,age+1) each callback: nine Katana role1
slashes. The right hand's 0x8000 parameter bit is masked as w_053 does; hitbox X stays 0x12.

Selected [Chakram/Vorpal blade/Crissaegrim](docs/player-weapon12.md) now own items133/163/164
through weapon12 as secondary roots in17..31: role0 GT4 slashes (blueprints50/52, descriptors1/2,
one mandatory row, program count D_8017BB28 in secondary_counter picks the shape, CLUT fade
retirement, timer9 via startup_ticks/SotnPlayer_SetWeaponStartup), role1 the blueprint56 Chakram
(rand speeds, accelerations, turn-limited homing) and role2 0x40 afterimages copying its palette.
All never consumed; roots/Chakram carry their items for CheckChainLimit.

Selected [Muramasa](docs/player-muramasa.md) now owns two-hand item141/kind29 through weapon56
(Gurthang's callback, own tables; descriptor2 under timer11; attack>=0x1E requests the
WFACTORY(56,0) Katana slash chain over tables 0xFCC/0xFA4, hand word 0x105C). func_8010A3F0 runs
before healing: two-hand Muramasa holds timer11 at 0x20 and counts blood updates
(`muramasa_blood`, committed after success); contacts clear each living update. Ordinary saves
are now v10: the u32 counter at 3632 (v9 migrates to 0). CalcAttack adds SquareRoot0(counter) in
its signed word; counters >=0x80000000 need SotnCharacter_BindExecutable. Case29: one crit rand,
attack (halved by poison) below 11 takes the rand()&7 roll. Older "v9" notes describe the v10
layout's other fields unchanged.

Selected [Heaven sword](docs/player-heaven-sword.md) now owns kind129 item119 through weapon13 as
a role0 secondary root (no rows): fly/stop, parabolic spin rise, turn-limited homing (step grows
0x20, never overshoots) and hitbox retirement; even g_GameTimer WFACTORY(0x3A,0) afterimages in
32..63 copy the raw parent's frame/childPalette/rotation with ENTITY_OPACITY fades. Never
consumed; only the root counts for CheckChainLimit. Dual Heaven swords' combo202 (left hand,
kind5, bp62 at <<14, chain1, five MP) runs w_013 role2/role3: the E2 sweep (screen words; no
CAMERA_LOCKED, so callbacks use retained_camera = camera - room_shift + screen_shift) arcs from the
hitbox, requests 0x46+(hand+1)<<14 strike swords (D_8017A5E0 params, counter advances without a
free factory), blinks, snaps to its target and reserves its one LINE_G2 row on the update reaching
0x34 (failure destroys; HeldRows, HAS_PRIMS). E3 strikes read the raw parent every update, launch
after its step3 and copy item/enemyId then. Mutation sweeps must use ctest's exit code.

Selected [Runesword](docs/player-runesword.md) now owns kind129 item143 through weapon14 hosted
by the secondary owner as role0: blueprint50 E0/F0 roots in slots17..31 (own item, least-used
group, 48 mandatory GT4s before RNG) fly/stop/drift/home and retire at the player's hitbox;
four 0x72 hitboxes copy the raw parent's id/enemyId and retire when its slot empties. The throw
is never consumed (`keep`); CheckChainLimit counts the root and initialized hitboxes. Trail
corners are world pixels (DRAW_UNK02|UNK_100). Secondary DestroyEntityWeapon(true) now retires
a raised slot16 shield (Moon rod too). SFX_RUNESWORD_ATTACK uses PlaySfxVolPan 0x50.

Selected [Moon rod](docs/player-moon-rod.md) now owns item132 and kind5 QCF special172: blueprint72
spawns five own-item E1/F1 crescents (player hitbox origin, least-used group, shield clear,
timer10=4, hit flash) whose sparkles chain from them. QCF honours limited CheckChainLimit via
per-hand special counts; kind5 starts only on the ground (substep0x45) and keeps the grunt.

Selected [Icebrand](docs/player-icebrand.md) now owns item113/kind22 QCF special181 through weapon50:
every pose1 requests FACTORY(0x61,descriptor) crystals; descriptor1 adds APPEAR_A/0x3A/0x40 at
birth and 0x44 frost at pose1. Sparkles lease eighty actual rows (child rows now 80) but keep
compact centre/UV/scratch state rebuilt at draw (SOTN_DRAW_WEAPON_SPARKLE); D_8017CBFC points live
in a program-BSS union shared with the crystals. Frost re-anchors outside the window as original.

Selected [Firebrand/Marsil](docs/player-firebrand-marsil.md) now own items111/117 and kind20/21
QCF specials179/180 through weapon49 and five secondary roles: sparks (parent attack copy,
func_80118894 group alone, orbit, ratan2 heading, ring request), the 16-wedge ring, puffs (private
anchors, six rand calls), swirls (phase2 CAMERA_LOCKED, pillar on %7) and pillars. Raw parents
may sit in slots16..47; E3..E5/F3..F5 are secondary kinds; bank2 frames at D_8017AA44.

Selected [weapon51 swords](docs/player-weapon51-specials.md) now own two-handed Claymore98/
Flamberge101/Zwei103/Obsidian107, Estoc95 (variant8 = D_8017AD58 past the table) and back-forward
kind23 specials182..185. Descriptors1..3 glow colour11 of their palette through program-BSS
words and real g_Clut writes, committed after the pass. Selected items pay HasEnoughMp(mp,1)
after the factory (back-forward: only if MP covers it); cramped Estoc starts no attack.
Blueprint54 replaces slot16 with the E1/F1 special (palette0x110+subType*2, pose1
WFACTORY(0x3E,0)); its fan root copies the raw parent and chains nine-callback GT4 blades.

Selected [Iron Fist/Fist of Tulkas](docs/player-fist-specials.md) now own items102/120,
kind24 rushes186/187 and kind25 fireball188: kind24 holds D_80139824 0x80 in substep0x58
(stand pose4->2; airborne Player_Jump hovers without transitions). Blueprint54 replaces slot16
with the E1/F1 rush (own item, hand group, 16 mandatory GT4s, lifetime128); descriptor4 requests
WFACTORY(0x3E,0) for the E2/F2 fireball, which copies its raw parent and retires after hitFlags.
Room entry translates quad-backed secondaries (ring/rush) per GT4 vertex, never as row storage.

Selected [Osafune/Masamune](docs/player-katana-warp.md) now own two-handed items139/140,
back-forward192 (root10) and kind135 warps193/194: blueprint60 replaces slot16 with an E2/F2
afterimage (factory-argument equip id) that slides out/back with SetSpeedX/DecelerateX, spawns
roots0x28 (Masamune birth)/0x1E (pose6, pose8 timer1) copying its item and enemyId0, retires at
pose15 and keeps screen position. Slash rows48/49 read past the table (row48 = live g_HandId);
original-data quads use the opt-in GPU 11-bit wrap/span rule. Kind135 enters sword-warp.

Selected [Thunderbrand](docs/player-thunderbrand.md) now owns item112/QCF special178 through
weapon48: birth requests root0x38/branches0x3A,1..3/ring0x40 before properties, pose-BSS bolts at
the first pose1 (0x10/0x20), kind19's forty-update pose0 hold (0x53..0x56 share 0x5C's handler).
Secondaries gain role2 (E2/F2) and slots17..63: lightning LINE_G2x10 with anchors/lifts/halved
extents, the GT4 ring (page0x1A, palette0x15F, THUNDER_B) and E2 row tags. Fire/Ice/Marsil stay
unavailable.

Selected [Katana](docs/player-katana.md) now owns two-handed item100/QCF special189
through weapon53: descriptors1/3 blend a quarter; descriptor1 requests WFACTORY(56,0) before
properties. Slash children own one mandatory GT4, roots0/10/20/30 (and 40..49) copy raw-parent
properties/enemyId, roots request WFACTORY(0x38,age+param+1) every callback (nine chained
children whose raw parent is the root's slot, even when reborn into it), CLUT schedules,
nine-callback lifetime and entity-relative DRAW_UNK02 corners. Osafune/Masamune own kind135 warps;
Yasutsuna has its own thrown owner (see its block).

Selected [Rapier/Were bane](docs/player-lunge-weapons.md) now own items23/22 and QCF
lunges191/190 through weapon38/39 and the shared weapon-secondary owner. Descriptor1 requests
WFACTORY(0x38,0) on pose4's final update. Kind26 stops both velocities, holds pose6 on a
forty-update D_80139824 and resumes substep0x50 at pose10/Down-Up tap/backward. Secondaries
retire outside step_s0x5A, reserve 24/32 mandatory rows, keep a per-hand program-BSS
countdown (0x640 at &7==7, 0x6F0 at zero) and Were bane's chain arrays; never saved.
Muramasa now has its own owner (see its block); its counter is saved in v10.

Selected [Holy sword](docs/player-holy-sword.md) now owns item115/QCF special196 through
weapon55 and the first native weapon-secondary owner: descriptor1 queues 0x69D then
WFACTORY(0x38,0) before properties; next-pass E1/F1 in slots17..31 reserves 64 mandatory
LINE_G2 rows before 128 rand calls, copies the raw slot16 parent's equipId/enemyId after the
least-used increment, shrinks its hitbox at frame1/timer1 and runs post-cull UpdateAnim.
Kind28 stands still, holds pose0 ten updates (D_80139824), plays 0x6EF without grunt and
charges five MP after the factory. Additive ABR1 lines; bank-2 table bounded at five entries.

Selected [Gurthang](docs/player-gurthang.md) now owns item121/QCF special195 through
weapon54: twelve optional LINE_G2 rows (28 birth rand calls) placed on trigger frames,
Dark descriptor2 with doubled attack, a 0x38 timer11 floor committed after the player
update and the FACTORY(61,0x11) outline. QCF resolution now precedes back-forward (grounded,
chain0x80, MP after the factory). Muramasa shares the callback (see its block).

Selected [weapon special inputs](docs/player-weapon-specials.md) now run the original
back-forward recognizer after QCF and resolve the equipped unk17 item after curse/
Alucard-sword QCF and before the actual factory. Basilard18's special174 uses weapon31
descriptor1, kind8 (no casting grunt); Holy rod/Star flail specials170/171 are kind4. Kind0 airborne variant6 needs Down plus Left/
Right. Other specials need their own states/callbacks; their items stay unavailable.

Selected [plain weapons](docs/player-plain-weapons.md) now own Morningstar129/
Mablung124/Holy rod130/Star flail131 and paired Takemitsu27/Nunchaku21/Red Rust26
through actual weapon3/57/4/5/36/33/35 programs running w_002's callback from their own descriptors. Preserve each
blend (additive; Red Rust transparent; Nunchaku none), Nunchaku's pose12 sound pair,
Holy rod/Star flail ANIME_SWORD_B, Star flail's pose6/7 widening, specials170/171,
Mablung descriptor3, Morningstar palette5 and live hand words in crossing palette
copies. Red Rust rolls one character libc rand after its actual factory; &7==0 takes
the curse failure branch without grunt. Great Sword127 (weapon51 descriptor0) uses DRA
bank10; Combat knife20 lingers four updates in slot16 and alternates with special175 via
the session-supplied slot16 item. Muramasa has its own owner. No
pool/grants/savev9 change; supplied stock/protection/position/relic/arrival/seed/save
fixtures are not acquisition/route/console evidence. All six Alucard gates remain open.

Selected [basic fists](docs/player-basic-fists.md) now own Knuckle duster85/Jewel
knuckles97 through actual weapon1 descriptors0/1, shared slot16 and four mandatory
GT4 trail rows reserved before flags/properties; exhaustion omits the attack without
RNG/retry. Preserve no sprite, per-row libc, camera-subtracted screen placement,
SDK sqrt/atan2/trig follow-through, NULL palette table, four-vertex rebases and draw
holds. Kind0 attacks use the original substep0x40 handler (pose7 combo, 0x24 stand-up). Iron Fist/Tulkas specials stay unavailable. No pool/grants/savev9 change;
supplied stock/protection/position/occupancy/curse/save fixtures are not acquisition/
route/console evidence. All six Alucard gates remain open.

Selected [basic shields](docs/player-basic-shields.md) now own Leather5/Knight6/
AxeLord8/Goddess11/Shaman12/Alucart167 through actual families8/9/11/25/26/58,
shared slot16, private animation/hitbox tables, palette copies with the live hand
word and native guarding. Only families52/58 cache/override attack1; basic shields keep hit-state4 attack255 and zero displayed/
GetProps ATK under buffs/poison. Any hit-state4 guard kills blockable projectiles
without DealDamage and otherwise rolls DealDamage, then discards it. Shield spells
and other shield families stay unavailable. No pool/grants/savev9 change; supplied
stock/protection/position/curse/save fixtures are not acquisition/route/console
evidence. All six Alucard gates remain open.

Selected [native MainGame tail](docs/main-loop-tail.md) now owns actual global
map-word advancement and one trailing native libc-adapter call after status time.
Preserve all paused/frozen/release/FFF/dead/GameOver calls, zero-count Rand, signed
normalization/wrap, pre-tail map packet phase, no draw/failure/pending-stage ticks,
atomic character HMS/RNG binding and no-tick connected/cold map-word transfer.
Startup consumes the same tail after actual title/menu/render/audio preparation.
No extra live pool/grants/savev9 fields. Fixed60 supplies256; native libc seed/LCG,
stage/combat RNG and saved-RNG restoration remain explicit adapters, not BIOS boot/
full sharing/raster/whole-pass parity. Source comparisons with supplied words/counts/
flags/cursors and native clock/protection/arrival/save-room/Fairy-load/failure fixtures
are not acquisition/routes/ordinary familiar activation/console evidence. All six
Alucard and full cold New Game/prologue gates remain open.

Selected [playtime/Badelaire](docs/player-playtime-badelaire.md) now owns actual
four signed status words, original single carries/hour cap and125 variant3 shared
hands. Preserve post-menu/map holds/closing-frame advance, frozen/release/FFF/dead/
GameOver time, draw/failure/pending-stage holds and birth-hour attack retention.
Native fixed60 supplies256, not actual SDK raster stalls. Keep engine/gameplay/HMS
services separate; stage phases and Wolf Blink use engine, subweapon phases use game.
Continue binds before gear/actor bootstrap; connected transfer keeps all four words
without ticks or legacy aggregate-ceiling loss. Preserve v9 legacy99999-hour import,
serialization ceiling without live mutation, signed pre-cap hour bonus and unbound
raw gates. Selected global map cursor/end-MainGame native libc-adapter call now run;
full BIOS/shared RNG/remaining timing stay open.
No new pool/grants/save fields; supplied clock/equipment/protection/arrival/save-room/
Fairy-load/failure fixtures are not acquisition/routes/ordinary activation/full-pass/
SDK/console evidence. All Alucard and full cold New Game gates remain open.

Selected [two-hand equipment](docs/player-two-hand-equipment.md) now owns ordinary
Namakura84 selection, one-copy/both-word reservation, displaced returns, real menu
confirmation and paired v9 restoration. Preserve selected-first return, one return
for old paired weapons, spare decrement before capped return (255 reselects254),
full169 order and virtual-empty exclusion without adding stock. Setter idempotence
is distinct from actual menu confirmation. Original shop capacity still counts both
words. Player/development bindings reject unpaired84 before grants; reread the first
commit before binding the second hand. Existing shared16/properties/graphics/forms/
rollback owners remain real; no extra pool/defaults/v9 fields. Explicit stock255,
protection/arrival/position/save-room/failure fixtures and adapters are not acquisition,
route/full-pass/console evidence. Other two-hand families,
remaining equipment/familiars/sharing and Alucard/cold New Game remain incomplete.

Selected [ordinary hands](docs/shared-ordinary-hands.md) now own actual shared16,
factory48/empty42/43, normal casting-grunt then property RNG, groups1/2, native
combat/rendering, Alucard-sword optional16 TILEs and shield phase4 knock-away.
Preserve FFFF-before-hitbox versus negative-timer-after-hitbox ordering, actual
unk48/preemption, original final shield drawFlags overwrite, double timer decrement,
allocation-before-particle RNG/no retry, same-family BSS, no-tick room/shove and
menu/map/frozen/draw holds. Alucard-shield displayed/GetProps ATK stays0; its
SetWeaponProperties override/attack1 remain separate. Commit form/spell retirement
after player success before the next real owner. Later effect rollback starts at
the post-player boundary; full-pass rollback remains open. Explicit stock/relic/
protection/position/occupancy/data/save-room fixtures are not acquisition/route/
console evidence. Selected [weapon0 swords](docs/player-basic-swords.md) now add19
owned one-hand items, real variants0/1/2 and actual palette-index/full384-word
copies. Preserve negative attack caps, actual defense/elements, Mourneblade orb-before-immunity and
existing deferred8HP contact. Selected Namakura two-hand and actual playtime/Badelaire now run. Secondary producers, shield spells, full equipment/sharing and
Alucard/castle remain incomplete. No new grants/pool/v9 fields.

Selected [Karma Coin](docs/player-karma-coin.md) now owns actual24 menu/hand use,
stock/Duplicator, shared weapon20 factories/children/26-row lease, private sprites/
G4/GT4 presentation and native physical/global damage with real Soul orbs. Preserve
chain1 before curse, player-success stock/grunt, mandatory26 allocation before root
RNG/shield callback and real failure retirement without retry. Birth sets neither
word10 nor12. Preserve floor-first/top correction/early return, private pose sound
volumes and actual coin-flip RNG. Tails owns word10=4/properties/internal UpdateAnim
hitbox words; heads owns word12=4/fixed4/hit82/effect2/group without a property roll.
Preserve high-only anchors, per-hand writable angles/closing-edge update order,
signed counter remainders and whole-word wrap, real84 scheduling/low parameters/
next-pass children, inherited raw bank/texture and parent existence every callback.
Same-family/room retains data; actual program/Axe Armor/full-stage/Continue reload
restores private data/BSS, never saves or transfers it. Native Wolf replacement
releases real rows before later allocation. Preserve shared sound-volume order,
Blink/clock/camera/shake, holds, real retirement and late-failure runtime/program/
row/character/RNG/player rollback. No new pool/grants/savev9 changes. Supplied
stock/relic/health/arrival/position/camera/RNG/pressure/failure/in-memory-save fixtures
and adapters are not acquisition/routes/whole-pass/full sharing/console evidence.
All Alucard/cold New Game gates stay open; ordinary Fairy UI remains disabled.

Selected [weapon15 stars/TNT](docs/player-star-weapons.md) now owns75..79 actual
menu/hand use, stock/Duplicator, shared roots/descendants/rows, private sprites/
animation and native damage. Preserve kinds129/130, actual50/68, chain3/TNT1 before
curse, player-success cost/grunt, timer10=4/conditional shield and raw AE/params.
Buffalo58 uses32..63, born9 lifetime/no inherited rotation; empty-parent blank
trail keeps wrapped priority65534. Flame80 uses25..31; common4/profile11 changes
factory entityId EF/FF only. Real weapon15 BSS holds same-family/room, resets on
actual program/Axe Armor/full-stage/Continue reload, never saves/transfers. TNT's
optional32 leases precede64 RNG; failure omits only visual, fixed1 attack/group/
SFX/word10/shield continue and never retry sparks. Preserve fuse absolute clock,
hit-byte2/3/return, collision order, spark words/hide ordering,76's split ranges/
low-byte increments/delay,78's scaled parameters and next-pass births. Properties
increment least-used before raw-parent group overwrite. Actual Wolf replacement
releases TNT rows; cleared body VX/VY/AE and group0 feed surviving real explosions.
Use explicit weapon15 secondary17/19 API; resident sleep17 stays resident. Preserve
raw-parent fields/no generation equality, private SDK, Blink/camera/shake/holds,
real retirement and native late-failure runtime/BSS/row/RNG rollback. State3/
scatter fixtures establish no ordinary producer. No new pool/grants/savev9 fields;
pressure/stock/relic/pose/position/failure/save fixtures and adapters are not routes,
whole-pass/full sharing/console evidence. Alucard/cold New Game gates stay open.

Selected [weapon16 throwing items](docs/player-thrown-weapons.md) now owns actual
80/81/82/134/135 menu/hand use, stock/Duplicator, shared roots/children/leases,
private sprites/animation and native physical combat. Preserve actual kinds129/130,
blueprints50/62/68/63 and chain3/3/2/3/4 before curse, player-success cost/grunt,
conditional shield/word10=4, knife allocation before properties and next-pass
trails58 in32..63/fire110 in25..31/common4-profile11 in32..63. Fire smoke changes
factory entityId EF/FF only. Preserve raw-parent fields without generation equality,
actual Wolf bank/body rotation/zero selfVX, private SDK/wrap/collision query order,
all24 eight-Iron impact requests and profile9 dust. Actual per-hand weapon16 BSS
holds through same-family menus/room cleanup; real program/Axe Armor/full-stage/
Continue reloads reset it, never save or transfer it. Menus/draws/spirit freeze hold
throwing/flame/profile11 owners; common smoke keeps its own original freeze flag.
Room cleanup precedes new allocation and retires projectiles/profile11/profile9
without a tick. Preserve real cull/Blink and native late-failure runtime/BSS/row/RNG
rollback. No new pool/grants/savev9 changes. Supplied inventory/equipment/relic/
protection/arrival/position/collision/failure/in-memory-save contexts and component
adapters are not acquisition/routes/whole-pass/global-sharing/console evidence.
US wet-smoke remains unsupported. All six Alucard gates and cold New Game/prologue
remain open; ordinary Fairy UI stays disabled.

Selected [cold title/Continue](docs/cold-start.md) now owns actual resident/SEL/title
startup before any character,17 real SEL SPRTs, private512x240/4:3 rendering,
four resident SPU banks/reverb and the explicit native filesystem Continue adapter.
Preserve original callback/input/engine-clock/fade/lightning/libc order, real row
retirement, draw holds and actual verified uploads. Native logo omission follows
the PC branch; it is not MDEC completion. Valid existing v9 saves alone create
gameplay; empty/corrupt/failed loads never fall back to a checkpoint. Prepare
candidate banks1/3 from the retained SPU without an audio render, carry engine
clock/held input, borrow then transfer the real save store once and ignore stale
departing-window focus. Failure retains published startup/SPU/store/save bytes.
No grants/savev9 change. Native filesystem labels are not original SEL UI; name
entry/New Game/Richter prologue/forest/return-to-title and the cold-start gate
remain open. Supplied save/focus/resource-failure fixtures and component adapters
are not acquisition/routes/whole-pass/full audio/console evidence. All six Alucard
gates remain open; ordinary Fairy UI stays disabled.

Selected [Magic Missile25](docs/player-magic-missile.md) now owns actual menu/hand
use, stock/Duplicator consumption, resident weapon22, shared50/56 roots/17..31
bow/arrows, private animation/rendering and native physical2 combat. Preserve
chain4 before curse using raw AE/params8000: rightF1 arrows retain params0 and
count left; cold arrows retain AE0 until actual raw-parent copy. Keep next-pass
birth, successful-root-only625, bow669/word10=4, conditional real shield removal
only after native success, raw parent without generation equality, actual Wolf
replacement fields/zero AE, no birth motion and only hitFlags1 impact. Impact
retains attack until original animation retirement/dead cleanup. Preserve callback/
real cull/animation ordering, full fractions/wrap, actual post-stage shove only
for bow40000, independent arrow camera flag, slot/engine-clock Blink80, menu/
freeze/draw holds, no-tick cleanup and real runtime/rows/character RNG rollback.
No new pool/grants/savev9 fields. Supplied inventory/equipment/relic/protection/
arrival/RNG/shove/failure/in-memory-save fixtures are not acquisition/routes/
whole-pass/full sharing/console evidence. All six Alucard gates and cold start
remain open; ordinary Fairy UI stays disabled.

Selected [native ability timer commits](docs/player-timers.md) now retain separate
original word10 (Bat Fire) and word12 (Neutron/Sire/Pentagram) birth requests.
Preserve actual indices, original4 overwrite, status1000, the other timer's count,
player-success/shared-owner commitment and ordinary countdown/menu/freeze/draw
holds. Actual player-entity cleanup clears consumed requests, not live player
words. Do not route weapon17/18/19 through the subweapon10 setter. No second timer
bank, grants or savev9 change. Supplied stock/protection/timer/resource/arrival
fixtures are not acquisition/routes/global timing/console evidence; all six
Alucard gates and original cold-start gate remain open.

Selected [Pentagram73/Bat74](docs/player-pentagrams.md) now owns actual menu/hand
use, inventory/Duplicator consumption, resident weapon17, shared50 roots/17..31
children,25 real rows, private GT4+24GT3 drawing and native global82 attacks.
Preserve actual item/hand chain1 before curse, player-success cost/grunt before6E4,
allocation before properties/RNG/effects,24 active callbacks, the transition's96
rolls/no projection and153 total callbacks. Use actual per-hand weapon17 BSS,
private SDK XYZ/GTE/AVSZ3 factor341, winding/degenerate retention and signed wrap;
never substitute per-child scratch, a second quad triangle or another pool.
Retain screen128/104 fractions, current shake once, clock palettes, menu/freeze/
draw holds, common frozen flash tail, no-tick room/weapon retirement and actual
factory/row/character/RNG rollback on late failure. Connected sessions retain BSS;
ordinary saves do not serialize it. No grants/savev9 change. Supplied stock,
equipment/protection/arrival/camera/RNG/failure/save-room contexts are not
acquisition/routes/whole-pass/global-libc/full-contention/console evidence.
All six Alucard gates remain open; ordinary Fairy UI stays disabled.

Selected [Power of Sire72](docs/player-power-of-sire.md) now owns actual menu/hand
use, inventory/Duplicator consumption, resident weapon18, shared50/56 roots and
17..31 parents/bursts, private animation/rendering, native global82 pulses,
flash5/6, shake3 and real profile12 particle leases. Preserve actual item/hand
chain1 before curse, casting grunt before birth66A, birth property roll and seven
later pulses only after successful actual root56 reservations. Keep old group
counts until end recount. Bursts retain raw parent slots/AE72 without generation
checks; read actual cold/replacement palette/texture, allocate before two RNG calls,
use private SDK trig, real post-callback culling and no animation-end retirement.
Preserve portrait screen retention, room-space burst motion, integer scale-only
pivots, white815F, final14 callbacks and real20+sentinel/no-RNG/double-decrement
profile12 retirement. Particle screen positions have no primitive camera translation.
Menus/freeze/draws hold child cursors; preserve the common flash tail through freeze.
Room cleanup adds no tick and actual late failures restore
factories/rows/character RNG. No new pool/grants/savev9 fields. Supplied stock,
protection/equipment/arrival/camera/RNG/failure/save-room fixtures are not acquisition,
routes, whole-pass/global libc/full-contention/console evidence. All six Alucard
milestone gates remain open; ordinary Fairy UI stays disabled.

Selected [Neutron Bomb71](docs/player-neutron-bomb.md) now owns actual menu/hand
input, inventory/Duplicator consumption, shared50 roots/17..31 children, one real
GT4, private weapon19 sprites/animation/SDK transform and one native global82
damage pulse. Preserve chain1 before curse: actual children already retain71 in
factory AE at birth, including the early halo; pending roots are excluded.
Preserve allocation-before-effects/properties, signed extent127..15..-1 before
hide, delayed GetEquipProperties roll, first-least3..6 group and next-callback
attack clear. Keep post-callback private animation, pose3/timer1 blend, pose5
opacity/unlock, signed scale-before-SDK-rotation/fade and real animation retirement.
Commit casting grunts before shared birth sounds, retaining earlier guard voice;
failed player updates queue no grunt. Project current shake once on halo and sprite.
Actual resident hand programs survive last-copy use; never infer weapon19 from an
empty hand or confuse its E0/F0 with food21. Preserve KEEP04000000 screen retention,
freeze/menu/draw holds, no-tick room retirement, Wolf replacement and actual late
factory/row/character RNG rollback. No new pools/grants/savev9 fields. Explicit
stock/equipment/protection/arrival/failure/save-room fixtures are not acquisition,
routes, full shared contention/global libc ordering/whole-pass/console evidence.
All six Alucard gates stay open; ordinary Fairy UI remains disabled.

Selected [owned food](docs/player-food.md) now connects42 items29..70 to actual
hand input, inventory/Duplicator consumption, shared50/56 roots/17..31 children,
private weapon21 sprites and raw deferred healing. Preserve item/hand chain2 before
curse, successful-player cost commit, extra GetEquipProperties RNG, signed wrap,
catch-before-motion, Peanut7B/anim0 mouth and pose3/timer8, Toad sim20 and delayed
raw81/power0/player-hitbox0, and independent Soul attack-child hitbox82. Meal Ticket
pickups use real inventory/sound and ordered shared-row text replacement; exhaustion
omits only text.
Preserve304 ordinary/frozen text clocks, menu/release/draw holds, real offscreen
retirement, no-tick rebases, actual menu weapon cleanup, resident artwork after the
last copy and late-failure UI/row/inventory/RNG rollback. No new pool/grants/savev9
fields. Explicit stock/HP/protection/position/arrival/failure and component adapters
are not acquisition/routes/whole-pass/full contention/console evidence. All six
Alucard gates remain open; ordinary Fairy UI stays disabled.

Selected [inventory-menu audio](docs/laboratory-audio.md) now queues actual
A7/A3/E entry andF/A4/A8 exit through the common sound ring. Preserve conditional
RR8 releases with retained20/22 IDs, private-definition rekey/gain, unconditional
A1/A2/A5/A6 clearing andA0 RR14/Init2. Stage22 gain retains parameters immediately;
queue1 uses the current ID at execution, including private entry0 after clearing.
Absent IDs queue nothing. Preserve duplicate-XA holds, ring wrap, post-script
ordering, ordinary menu player/game holds and no-tick draws. Map and spell-freeze
are separate owners. Pair/gain adapters and explicit native audio/card/load/arrival/
protection fixtures are not acquisition/routes/fullSDKflush/console evidence.
All six gates remain open; no new pool/grants/savesv9 or ordinary Fairy UI.

Selected [direct-voice ownership](docs/laboratory-audio.md) now retains actual four
12..19 effect identities/priorities, same-ID/free/strict-higher/equal arbitration and
open-SEQ18/19 restriction. Equal replacement intentionally excludes group3 outside
SEQ. Mirror actual rendered ENV words and reclaim direct/script leases BEFORE
effects; decoder-active/ADSR-state flags are not SDK key-status substitutes.
Preserve copied scripts and distinct group3 key-off12..15/playback14..17. Script
notes use the held pre-effect status cache, first-even free test and one shared
KeyOnRange fallback cursor, even across ranges; retain cursor/cache through mute.
Retain20/21 and22/23 IDs/gain/pan, A1 ID-clear/RR14, mode2 RR8 and Init2 versus Init1
reset boundaries.
The selected SDK software key mask initializes zero; its note-on flush writes
hardware directly and this owner only clears the mask. Other SDK key producers,
full flush/NTSC timing, voice pressure/global controls/session sharing and all six
milestone gates remain open. Pair/gain adapters, ENV/cursor fixtures and supplied
native audio/card/protection/arrival/tone pressure are not ordinary acquisition/
route/console evidence. No new voice/entity pool, grants, savesv9 or Fairy UI.

Selected [common CD bootstrap](docs/familiar-cd-audio.md) now owns actual CD/SPU
binding before any familiar load, private stage-profile requests, flag20 silence,
real command4/read acknowledgements and no-card game-over audio. First bank2 load
validates then rebinds the same CD to the prepared SPU before old-owner retirement,
retaining current/saved decoders, queues, voices/DSP and physical effect-ring cursors
without a tick. Save-room entry queues actual stopA; exit repeats stop until actual
XA activity clears before the stage request. Never recreate a legacy decoder.
Clock-room stop is not inferred from save-room evidence. Stage/card/resource/audio/
edge fixtures and bounded instruction blocks are not ordinary acquisition/routes/
full engine-room-fade/console evidence. Full cross-session audio/SPU continuity,
remaining global controls/voice sharing and all six milestone gates stay open;
no grants/pools/savesv9 change or ordinary Fairy UI.

Selected [effect-ring ownership](docs/laboratory-audio.md) now preserves the original
256 read/write slots, last matching effect, retained volume/pan and wrap-to-empty.
Consume IDs before lookahead; preserve physical cursors and stale volume/pan words
through ordinary updates and prepared SPU replacement, clearing them only at actual
MuteSound initialization. No queue-capacity failure or replacement voice pool.
Standalone controls/ambience retain their compatibility API; bound controls use the
real CD sound ring. Explicit burst/arrival/protection fixtures and dispatch adapters
are not ordinary producer/acquisition/route/console evidence. Global bootstrap,
voice allocation/key timing, session continuity and all six milestone gates remain
open; no grants, savesv9 changes or ordinary Fairy UI.

Selected [bound game-over audio](docs/laboratory-game-over.md) now consumes actual
death128 through separate Engine81/Init11 frames, acknowledgement-plus-XA gating,
12/B cleanup, actual MuteSound metadata/transport retirement and managed33e. Queue
emptiness is a separate flag. Keep rendering clocks held until the real resident
resource/audio phases; never invent a ready/fade timer. Bound game-over audio keeps
actual SPU command/render updates, no overlapping legacy decoders and no revival.
Mute requires closed SEQ, cancels actual current/saved/in-flight CD transports and
clears original queues/attachment/current metadata while retaining voices, DSP/RAM,
registers, copied scripts, restore byte and original retained XA words. Controls
never add a DSP/sample tick. Explicit card/load/lethal/HP1-water-result/audio fixtures
are not ordinary acquisition/water routes/console evidence. Unbound bootstrap,
full entity/GPU cleanup, session/title/whole-pass and all six gates remain open;
no new pool/grants/v9 changes or Fairy UI.

Selected [bound SPU/CD sound ring](docs/familiar-cd-audio.md) now queues actual
7/E/F/A1/202/204 with XA/master controls. Preserve original duplicate-XA early
return holding later SPU commands, script-update-before-command copying, resume
waiting for the next script update, attached SEQ identities and pre-command lease
span. Script gates follow the retained SEQ access after EOT until explicit close.
Actual A1 clears RR low6 bits including exponential release on20/21. Original
InitSoundVars2 clears the pause flag but keeps copied scripts and pending effects.
Native unbound/standalone paths retain their existing selected APIs; bootstrap,
game-over/global sound ordering/session and all six gates remain open. Explicit
cursor/copy/register/SEQ-access/actualSFXqueue/card/arrival/protection/event/tone
fixtures are not ordinary producers/acquisition/routes/console evidence. No new
pool/grants/v9 change or ordinary Fairy UI.

Selected [bound master fades](docs/familiar-cd-audio.md) now own original80..84,
command10, actual final native gain, depth0, StopSeq and three-update all24 RR8
release/cleanup. Preserve shared80/90 multipliers, signed counters, immediate single
fade flag and actual CdFadeOut1 queue3/reset127, including no-playing reset. Original
StopSeq closes an access retained after EOT and releases all24; EOT itself releases
only SEQ notes. RR writes clear low6 bits without clearing envelope/decoder/RAM;
current scripts/allocations clear, paused copies and pending effects survive.
Native final mix applies master once after SPU/CD/legacy additions. Explicit
arrival/card/load/protection/events/tone/SEQ-access fixtures are not ordinary
routes/acquisition/console evidence. Bootstrap/gameover/globalring/CDwet/session/
persistence/whole-pass and all six gates remain open; no Fairy UI/pools/grants/v9 change.

Selected [CD/SPU depth ownership](docs/familiar-cd-audio.md) now binds the actual
configured sound owner, applies mode0 depth127 before6 and restores the original
current byte at6 completion. BSS restore0 is separate from SoundInit SDK depth10;
actual SEQ changes the byte. Rebind prepared sound before retiring the old owner.
Preserve command-update/CD/render order and pre-SEQ script lease span, including
fourth-channel retirement when SEQ ends. Binding/depth requests never tick DSP.
Explicit clock/tone/owner fixtures are not normal traversal or console evidence;
normal unskipped service cadence remains separate. CD wet/unbound master/remaining global
mix/order/session/persistence/whole-pass and all six gates remain open. No new
pool/grants/v9 changes or ordinary Fairy UI.

Selected [native SPU voice reverb](docs/spu-reverb.md) now owns private SDK Studio B
coefficients, actual tone mask0x4 sends and the existing SPU RAM work area. Preserve
half-rate/44.1kHz phase, signed-word depths, stereo IIR/comb/APF work, cursor wrap,
write-disable reads and zero-depth processing. Validate actual VAB reservation before
clearing/uploading; familiar preparation retains the live DSP and work RAM. Native
SoundInit depth10 and selected private SEQ depths now apply. Audio updates continue
through menu/spell freeze; draws/load preparation never tick it. Explicit tone/card/
protection/reference fixtures are not ordinary effect/acquisition/route/console
evidence. Remaining CD sends, other presets/unbound master/mixer/session continuity
and all six milestone gates remain open. No new pool/grants/v9 changes or Fairy UI.

Selected [bound stage CD routing](docs/familiar-cd-audio.md) now shares actual
music/speech/stop/pause/restore and private-profile restart with the existing CD
owner. Preserve pause-before-same-event-speech, single combined stop, original XA
activity/both acknowledgement flags and immutable post-stage dialogue snapshots.
Never create overlapping legacy decoders for that binding. Laboratory/Library90
and shared dialogue92 retain original identities; Laboratory victory during active
state3 increments to4 before5. Explicit audio events/activity/retained music words
and card/protection fixtures are not encounter-route/acquisition/console evidence.
Unbound bootstrap/game-over, CD reverb, other collapsed stage requests, full
sound-ring sharing/session continuity/persistence/whole-pass and Fairy UI stay open.
No grants/pools/v9 changes; all six milestone gates remain open.

Selected [CD volume fades](docs/familiar-cd-audio.md) now bind native stage90..94
to actual command8 and real decoding. Preserve the immediate separate fade flag,
signed words, shared128/8/4/2/1 multiplier, possible gain increase after retarget,
single-flag clearance and deferred stop2. Original xa_play_ck is pending fade OR
playing identity, never decoder readiness; ordinary/frozen/FFF actor snapshots use
that owner. Loop6 may reconstruct its retired decoder but never enables serial mix.
Menu/spirit-freeze audio advances on actual engine clocks; draws hold. Supplied
stage-event/card/protection/music fixtures are not ordinary script/acquisition/route
or console evidence. Unbound master routing, general stage music/dialogue, CD reverb, persistence
and whole sharing remain open; no grants/pools/v9 changes or ordinary Fairy UI.

The [CD-command service](docs/familiar-cd-audio.md) now reconstructs selected
managed mode1 continuation and mode0 command6 with actual private music transports.
Preserve original playing identity, strict wrapping-clock expiry, metadata-only
continuation without PCM/history reset, real loop Setloc/ReadN and pending12-to14/6
ordering including255-entry overflow. Preserve mode/transport bookmarks and raw<40
music's abandoned-position reset. Actual PCM exhaustion flushes its tail and holds
silence until real seek/stop without changing command state or reading unrelated
tracks. Adopted legacy loops and finite speech retain their own behavior. Changed
global queued-base retargeting fails explicitly; general native stage/dialogue/CD
session ownership, reverb and whole sharing remain unfinished. Wider301..532 service
inputs are not additional controls. Explicit expiry-clock fixtures and private
audio-service cadence checks are not stage-route/menu/acquisition/console evidence.
No grants/pools/v9 changes; ordinary Fairy selection and all six gates remain open.

Selected [engine/session familiar initialization](docs/familiar-lifecycle.md) now
derives the actual active card, binds original mode0 after native bootstrap/64
AluInit RNG calls, and transfers seven records/selection without a menu load.
Retain cached overlay history/events/sound cursor/generation subject to original
mode0 writes, replace entity generations and preserve departing owners on candidate
failure. Cached-none creates no root; later actual menu reselection increments once.
Flag20 engine entry omits new stage XA. Existing v9 bytes remain unchanged and do
not persist familiar records; in-memory handoff is separate evidence. General CD
bookmark/queue/dialogue/cutscene lifecycle, persistence, ordinary menu activation
and whole-pass acceptance remain open. Supplied card/reward/arrival/save-room
fixtures are not acquisition, uninterrupted-route or console evidence.

Selected [connected-room familiar ownership](docs/familiar-lifecycle.md) now binds
actual flag20000 retention, temporary-row retirement and old/new player screen
displacement before the destination stage pass. Preserve fractions, root/wings
generations/cursors, history/events/audio/progress, no extra callback or load count,
and no duplicate destination camera shift. Ordinary Engine_3 does not call
ServantInit; mode2 belongs to its separate special return. Initialized medicines
retain their original handoff while cold children retire. Supplied edge/camera/card/
item/health/protection fixtures are not uninterrupted-route/acquisition/console
evidence. Remaining stage/session/persistence/general audio and whole-pass gates stay open.

The [selected familiar load owner](docs/familiar-lifecycle.md) now binds owned
0/3 through the native menu/load-boundary API: all four verified resources,
detached resident graphics/SPU candidates, root4/wings5 and empty-CD preparation
precede real retirement/publication. Preserve old owners on failure, complete
namespace validation, actual player snapshots, decoder/resampler position and
all live voices/scripts/queues. Same selection does not reload/increment; cached
reselection increments once without replacing resources; deselection retains
loaded cache/audio/history and unrelated shared effects. No grants, frame ticks,
RNG or v9 change. Ordinary menu activation, room/stage/session binding, persistence,
general audio/cutscene handling and whole-pass acceptance remain unfinished.
The first UNCAPTURED death interruption can finish an expiring animation tick;
retain actual captured textures and subsequent17 holds, without forcing prior
uncaptured artwork equality. Supplied card/item/HP/protection/load-boundary fixtures
are not ordinary selection, acquisition, route or console evidence. All six gates stay open.

The [selected familiar retirement](docs/familiar-lifecycle.md) now validates the
complete actual5..7 DD/DF namespace before clearing4..7 or translating rooms.
Preserve every real row/sentinel/generation and reject absent-lease orphans before
any sibling retires. Earlier row0-only translation prechecks allowed partial late-
sibling retirement; the full validation fixes that failure. Clear retains overlay
history/events/sound cursor/serial/binding and unrelated stage/common/shared rows;
empty retained namespaces skip native provider queries. No clocks/RNG/audio/scene/
resources/progression change. Actual card/resource initialization, deselection,
room/stage/session lifecycle, persistence and whole-pass rollback remain required.
Explicit leases/effects/owned-item/protection/lethal fixtures are not ordinary
selection, acquisition, route or console evidence. All six Alucard gates stay open.

The [bound native Fairy dispatch](docs/fairy-native-dispatch.md) now runs actual
4..7 callbacks and a distinct full-player FFF engine branch: Life Apple spending,
real effects/cleanup/resources, release-only frame and Blink4A. Preserve stable
birth gates, held gameplay/shared8..63/recount/shake, continuing engine/HMS/
audio time, captured death texture/generation and separate dead resource writes.
Seven actual signed-word familiar records now belong to the character. Raw enemy
rewards update the selected record transactionally, with player99/zero/no-selection
holds, bit-length minimum1 and EXP9899 cap. Loads increment only on changed/reloaded
selection, cap9999; ordinary character restore retains records. Existing v9 layout
is unchanged and does not persist them. Ordinary card/resource initialization,
room/stage/session lifecycle, persistence, full audio/cutscene bindings and whole-
pass rollback remain required before enabling the Fairy menu. Supplied card/Apple/
root-init/protection/lethal fixtures are not acquisition/selection/persistence/
route/console evidence. All six Alucard gates remain open; no new grants or pool.

The [bound Fairy rescue response](docs/player-fairy-rescue.md) now supplies gated
17 interruption/hold/resource/recovery and a dedicated real Blink4A producer.
Preserve numeric drawflag4 for BOTH physical/water interruption, resident captures/
pivots/serials, full dead callbacks at the held actual gameplay clock, all16 word/
pending-heal/dead-status holds even after HPMP restoration, distinct release-only
frame and pre-collision factory origins. Keep hurt-owner guards separate. Clear
active death presentation only on actual normal recovery; retain first-frame
rotation and persistent entity blend until actual normal-reset/form owners clear
it. No invented palette reset, generic revival or optional substitute effect.
Production nativeFFF/Fairy selection/progression/item/lifecycle/whole-pass bindings,
remaining stone/grab/Axe contexts and all six gates remain open.32 states have
selected native producers plus gated17; supplied scene/resource/clock/contact/HP1
and composed native loops are not ordinary activation/rescue/acquisition/routes/
console evidence. No default grants/savev9/pool change.

The [Fairy rescue-cleanup services](docs/familiar-rescue-cleanup.md) now retire actual
shared factories8..15/children16..63/Wolf rows and native projectiles. Validate the
whole committed runtime and every real row/generation/sentinel, reject stale/orphan
ownership before mutation, require stage-mark acknowledgement and consume native
capture/audio/notifications first. Preserve common8/environment rows, AluInit
vectors/RNG, shared Blink locks/phases, global points/cursors/serials and attack-group
counts until ordinary recount. No callbacks/clocks/healing. Source0x29 is decimal41
STOPWATCH, never decimal29 Hellfire flash; clear freeze only with an actual clock.
Retain projectile map/pause/serial/clocks/queued sounds. Native Fairy/FFF/player17,
resource restore, Mist/other owners/full namespace/whole-session rollback remain
required. Supplied directcleanup/effect/gear/protection/rawcontact/capturedphysical
death contexts are not actual rescue/acquisition/routes/console evidence. No pool,
grants/savev9 change;32/34 states and all six gates remain open.

The [familiar inventory/translation services](docs/familiar-inventory-translation.md)
now own actual unchecked bag-byte consumption, no-tick camera/player/room slot
operations and finite Blink4A. Preserve Duplicator/equipped-copy independence,
delayed0-to255 wrap and ordinary pickup cap; raw hand bytes already fit unchanged
savev9. Body cap/order/reserved/version checks remain. Preserve independent entity
camera/player flags, packet2-before200, subtraction of floored original player
displacement, sentinel translation and real room20000 retirement after validating
all sibling leases. No callback/RNG/audio/timer ticks. Blink4A is no revival owner.
Actual gameplay/lifecycle inputs, transactions, progression and FFF/player17 remain
required. Supplied flag/origin/inventory/image fixtures and bounded original engine
fragments are not ordinary activation/acquisition/routes/console evidence. No
pool/grants/save layout change;32/34 states and all six gates remain open.

The [Fairy slot/effect/render runtime](docs/fairy-slot-runtime.md) now owns actual
4..7 identities/generations, root/wings initialization, local-before-room event
births, live DE/DC callbacks and real DD/DF stage leases. Preserve fresh original
sound-step99, same3 retention/cache reset, actual parent generation and stable
pass gates0/3/FFF; the latter is not full native FFF/player17. Preserve full
fractional origins, first-empty5..7, optional failure and actual55/arg2ZERO roots.
Manual hand restrictions/arguments remain separate. Validate all previous lease
rows/generations, including the hidden sentinel, before any replacement; preserve
unrelated stage/common/Bat/Wolf rows. Scene drawing uses real bank20, sorted
priorities/clipping/shake once and selected untextured G4 DTD. No draw ticks.
Inventory, whole-pass native commit/rollback, gameplay/card/resource lifecycle,
camera/room, saved progression and rescue remain required. Supplied snapshots,
level/load-count/init/gate/origin fixtures are not ordinary activation/acquisition/
route/console evidence. No pool/grants/savev9 change; all six gates stay open.

The [familiar CD audio service](docs/familiar-cd-audio.md) now owns original
sound-ring/CD-queue256, immediate10/11 ACK-busy, actual decoder-backed start/read,
pause/two-position restore and finite mode2's20-tick tail/strict expiry/stop phases.
Preserve both queue overflow/clear rules, duplicate early return, gain changes12,
third-save wrap, actual retained decoder/history/converter samples and actual
engine clock. Decoder completion is not XA-playing or ACK. Native stage intro/
loop retains its existing service; original CD mode0/1/hardware locations/latency/
interpolation are not established. Read failure must never supply fake readiness.
This does not yet bind the native Fairy event/4..7/selection/engine-session owners
or complete rescue. Supplied DE/room/cutscene and decoder tests are not ordinary
activation/acquisition/routes/console evidence. No grants/savev9/pool change.

The [Fairy item effects/speech](docs/fairy-effects.md) now reconstruct DF's actual
eleven profiles and DD's ten-G4 light, plus existing-decoder playback of eight
original US XA clips. Preserve129/33 real-row requirements, sentinel ownership,
allocation before RNG, whole-word particle versus fractional entity origins,
128 one-per-update activation, raster high-word rebinding, signed wrap and exact
birth/lifetime/retirement boundaries. DD's435 packets omit camera2: do not shift
its birth fan twice. No new pool, completed leases/healing/rescue or CD ACK is
implied. Actual native4..7/lifecycle/rows/FFF/player17/cleanup/progression remain
required. Private origins/allocation/decoder checks are not activation/acquisition/
route/console evidence. No grants/savev9 change; all six milestone gates stay open.

The [Fairy control components](docs/fairy-controller.md) now reconstruct selected
all US root/wings callbacks plus shared sound-state/49-record room-event components.
Preserve screen/camera ownership, hint omission of ability selection, original intro/
sound-step/pause requests, shoulder locks/animation-result gate and US non-Yousei
boundaries. Actual CD queue/ACK/XA inputs are required; commands are not playback.
Preserve room/servant cache keys, stage-alone hold, camera/condition-gated timers,
delay-minus-one, first-empty5..7, one-shot consumption on exhaustion, repeating
reloads and cache-only resets. Plans do not establish real slots or hint availability.
Preserve original private
tables/libc RNG, death-first selection, late potion urgency overwrite, pointer-based
animation, source movement/SDK math, Hammer's omitted39 gate and item consumption
despite optional failure. Preserve carried25/91-frame Life Apple counters and
separate resource/FFF/release requests. Requests do not enable Fairy or rescue17.
Native player snapshots now own actual transient damage effects and pre-control
step/substep: clear on full successful player updates; write responses1/2/3/7/8,
including zero-HP stone7; immune/absorb/water/death4/6 leave zero. Hold freeze/menu/
draw/rebase, preserve failures. Actual events/CD-XA/native4..7/effect
rows/FFF/player17/cleanup/progression remain required. FFF runs full player and
only familiar4..7, never shared8..63; Spirit freeze3 is not a substitute. Private
callback/attribute/raw-contact/HP1 fixtures are not native familiar, acquisition,
route or console evidence. No pool/grants/savev9 change; all six gates stay open.

The [familiar foundation](docs/familiar-foundation.md) now supplies verified US
TT/FT/VAB resources, original shared animation, stage-bank20/resident FT+CLUT and
existing bank2 sound binding. Preserve zero-load/single-jump/transfer/end-hold,
signed timer wrap, retained16-bit offsets without properties and no-tick drawing.
Preserve full256-word Fairy palette copy within the private overlay, unused null
sprite indices as unavailable, validation before uploads, unchanged unrelated VRAM,
clear-binding/resident retention and bank2 voices/scripts/queues/other-bank state.
No native familiar lifecycle/slots/activation/combat/progression, Fairy item producer,
Life Apple17 or Sword Brothers39 is established. Voice/sprite/facing/property checks
are fixtures, not acquisition/routes/console evidence. No pool/default grants/
savev9 change; all six milestone gates remain open.

Selected [owned medicines](docs/player-medicines.md) now connects the17 equippable
items144..162 excluding146/147 to ordinary input, actual55/rowless39, private
schedules, real circle53/38/32 rows, outlines/Blinks/two-GT4 labels and resources.
Preserve before-curse input, actual39 slots32..63 gate, root exhaustion without
cost/replacement, successful-player consumption, Duplicator84 and spare semantics.
Commit resources after real rows:4096to4095, cure words2, kind1 pending overwrite,
Mail later overwrite and Ankh80 one50% bonus even with two. Bind full MP to both
actual character/player, including dead; pending healing never revives death.
Original menu forbids Life Apple146/Hammer147 hand equip/use; Fairy/state17 remain
unfinished. Hammer's separate signed DRA word/protocol owns64, actual stone11
shake1/forced taps, full-player decrement, freeze/menu/draw holds and post-bootstrap
session retention, never saves. Preserve circle flags/SDK/angles/optional exhaustion,
hidden suffix camera2 and no-tick room/shove boundaries. Prune all actual root
records by20000 before destination allocation; retain initialized20000 circle/Pink,
retire cold38/39. Other child cleanup/whole-native rollback and all six gates stay
open. Owned gear/HP/MP/ailment/clock/factory/position fixtures are not acquisition,
uninterrupted routes or console evidence. No pool/default grants/savev9 change.

Selected [room callback/cleanup ownership](docs/player-room-ordering.md) now runs
ordinary departing player/DRA/stage/HUD/audio before room lookup. Actual spell
freeze/teleport warp preempt lookup; destination initialization adds no second
player/shared effect/game-clock/audio frame. Prune actual32/33/65 and selected
39/46/121 roots before new actor allocation; retain attached20000 Ice24 row indices,
retire detached ice/dissolve/teleport and commit real row plans before runtime.
Clear departed raw contacts only after successful entry; hold healing/all16 words/
HP/RNG/frame. Preserve callback/cull-before-camera and ordinary offscreen retirement.
Other cleanup/fade/engine timing/whole-native rollback and all six gates remain open.
Explicit contact/effect/protection/edge/camera-history fixtures are not acquisition,
uninterrupted routes or console evidence. No pool/default grants/savev9 change.

Selected [Library Card](docs/player-library-card.md) now owns ordinary166 hand use,
one actual consume, Duplicator84, player18/F4-F5 and actual Library travel/arrival.
Preserve Card-before-curse ordering, grounded normal states, birth velocity, held
healing/player words/input, padSim0/demo4, real eight-root pose8 retry and36-row
retry-before-RNG. Consume only after player success; native failure restores actual
reservations. Preserve marker31 versus actual departing stage,64-vector RNG before
incoming121/high1, left arrival, global point retention and terminal ACK recovery.
Actual FFFF/2 white engine tiles advance once per prepared engine frame, never on
repeated draw. Retained DRA attack fields clear after callbacks on outgoing0/death;
do not substitute temporary combat masks. Menus hold actual player/effect/game
clocks. TOP/RTOP triggers/stages, wider sharing and all six milestone gates stay open.
Owned gear/protection/failure/root fixtures are not acquisition/routes/console.
No new pool/default grants/savev9 change or whole-stage atomicity claim.

Selected [shared teleport effects](docs/teleport-effects.md) now own actual
factory121/child65/36 rows, one32-point global and original96-call births. Preserve
allocation retry before RNG, private gradient/UV words, raw packet delay/animation
bytes, opaque particles, top-Y clamp and first-GT4-only hide bugs. Preserve actual
TILE/G4/GT4 ordering, flags10000 frozen advances, release/menu/draw holds and no-tick
rebases without translating shared points. Actual room transitions retire child65
and nonretained pending121 roots before new actor allocations, without ACK/RNG.
Other shared room cleanup and whole-stage atomicity remain unfinished.
Terminal ACK/sound636/row retirement
precedes later work; palette/warp outputs are intents until their real owners commit.
Selected Library Card now supplies its player18/Library travel consumer. No other-
castle route, pool/grants/savev9 change is established.
All six Alucard gates stay open. Factory/protection/freeze contexts are not ordinary
acquisition/routes/console evidence.

Selected [grab control](docs/player-boss-grab.md) now accepts retained reaction0/2
and actual boss-supplied release4 velocities. Preserve normal reset with retained
stone animation/no entry8/voice, pulse4/8161/no normal voice, default DAMAGE's
retained reaction, release2E/hurt15/6ED/raw impact word and no immediate HP loss.
Release motion belongs to the boss, never inferred from facing. Preserve actual
secondary wall damage and failed-animation pending requests. Current Hippogryph
still clears to drop; throwing-boss/stone producers, dead release and broader
contexts remain unfinished. Explicit grab/reaction/velocity/protection fixtures
are not ordinary encounters, acquisition/routes/console evidence. No grants/v9 change.

The [stone image owner](docs/stone-dissolve.md) preserves the separate common-death
cursor/count across actual entity32 StoreImage, patterns and shards. Capture copies
pixels only; actual physical/water capture alone resets common decay. Primary and
secondary rectangles follow their own last committed upload, not later scratch
mutations. [Common knockback](docs/player-hurt-motion.md) narrows center delta to
signed16 before strict16/velocity/sign tests; surviving17/18 force sides, lethal
17/18 recompute ordinary source direction. Preserve default grants/savev9 and all
open gates. State17's real producer is Fairy Life Apple rescue, not a generic fade.

Selected [physical/elemental death](docs/player-physical-death.md) now owns actual
normal/fire/thunder/ice C0 births, shared flash/Blink/children, resident5120 capture
and64 committed real uploads. Preserve element priority, original recoil/deceleration,
birth flags/pivots, deferred BP51, high-parameter thunder/death ice delay, byte-color
underflow, signed velocity/rotation and actual animation22 turn. Native capture
uses the existing scratch/shared generation, never the unbound legacy CPU mask.
Preserve frame8000 upload-skip, SDK body/Blink angles and pivots, no artwork reupload,
pending dead healing and actual game over. Failed player updates restore real
texture words/validity, scratch/generation and factory reservations; no whole-stage
HP/clock/audio atomicity claim. Preserve optional allocation before RNG, frozen/
draw holds and separate water/stone entries. Raw-contact/protection/prospective-clock
fixtures are not ordinary lethal encounter/acquisition/routes/console evidence.
Remaining damage/grab/Hammer/fade/full sharing and all six Alucard gates stay open.
No pool/default grants/ordinary savev9 change.

Selected [stone control](docs/player-petrification.md) now supplies actual queued
entry text3, ordered landing0/39/smoke3-or7, shake1, ordinary/rare wiggle factories,
forced directional taps and actual palette/SDK flags. Preserve the heavy helper's
647 before644, third coincident644/608/6E7 call, source fractions, airborne
protection4 without a ceiling clamp and unconditional rare protection4. Escape
selects3B before the live comparison, including rare smoke3. Lethal already-stone
checks actual child32 slots16..63/phase0, never pending factories; absent children
delegate selected physical C0. Preserve new-lethal32 wait, death80/five forcedUp,
hidden810D/3E/smoke3 and numeric entity flag4 rotation. Actual zero break sprite
copies secondary then retires32/36 rows before motion; never fabricate shards or
48 uploads on this branch. Raw-contact/protection/RNG/cleanup fixtures are not
ordinary Medusa/acquisition/routes/console/room transfers. Stone grabs/Hammer,
remaining death/fade/full sharing and all six Alucard gates remain open. No pool,
default grants or savev9 change; no whole native HP/motion/audio rollback claim.

The [shared stone dissolve owner](docs/stone-dissolve.md) now accepts actual
factory39/child32 with36 real GT4 rows, previous-resident capture and private
primary/secondary uploads. Preserve allocation retry before37 birth RNG, the one
shared5120-byte scratch, eight-update patterns/forty calls, frame8000 upload-skip
without body hiding, engine-clock modulo3/living or2/state16,48 committed decays,
signed delay/motion and terminal retirement before resets. Preserve actual SDK
geometry, no-tick holds/rebases, scoped room cleanup and real two-rectangle words/
validity rollback after late upload failure. New water capture generations may
reuse the scratch. Effect rollback is not whole-player HP/motion/audio atomicity.
Supplied zero-petrification/factory/protection/HP1/lethal-result fixtures are not
actual stone producers/death/acquisition/routes/console evidence. No new pool,
grants/savev9 change; selected stone control now supplies the producer, while
remaining stone contexts and all six Alucard gates remain open.

The [shared stone-effect support](docs/stone-effect-support.md) now accepts actual
text3, smoke3/7, particles3/8 and entity24high0 through the existing shared runtime.
Preserve three-call smoke births, random2..5 index versus retained full parameters,
private normal-size high0/two RNG versus Hellfire high1/one RNG, particle3 behavior2
(NOT vectors),8/30 actual tiles plus sentinel,47/63 double lifetimes, hidden delay
wrap, whole-word origins and live facing. Existing row-index capacity31/draw stride30
are not new pools. Keep allocation-before-RNG and actual rendering/lease retirement.
US status20000 smoke remains unsupported. These controlled factory owners do not
by themselves establish dying-player,
acquisition/routes/console evidence. No grants/savev9 change; all six Alucard gates
remain open.

Selected [HUD HP hold/secondary impacts](docs/player-hud-hp-hold.md) now owns actual
DRA64-if-zero displayed-HP delay and actual save-room16 overwrite. Preserve
same-frame63/15,65th/17th interpolation, original producer exclusions/full-HP
healing, ordinary/frozen/release updates, menu/map/draw/GameOver holds and no-tick
room retention/fresh-HUD reset. Commit notifications after player success, never
replay them in freeze. Kind2 grab/secondary share prior-poison status and inclusive
Talisman work; secondary sign-extends its retained16 before division and always
notifies/displays guard/dodge/lethal results without contact guard/dodge effects.
Preserve exclusive provenance, pre-minimum zero/negative words, wall644 before
voice, actual Cloak/Fury and expiry-protection gates. No pool/grants/savev9 change.
Supplied contact/status/gear/protection/arrival/heal fixtures are not producers,
acquisition/routes/console evidence. All six Alucard gates remain open.

Selected [held-grab damage feedback](docs/player-grab-damage.md) now owns actual
normal held kind2 defense/prior-poison/Talisman/HP work, unconditional type0 numbers
and surviving Blood Cloak/Fury requests. Preserve poison-expiry status, one inclusive
roll for one/two accessories, pre-minimum signed-word dodge numbers and full lethal
overkill. Grab guard/dodge keep held/palette4/sound6E7 without contact guard/dodge
visuals. Preserve captured pre-pin fractions, old deferred healing priority, actual
entity19/twenty rows, zero/full-word provenance, optional exhaustion and failed-player
rollback. Cloak full suppression, longer Fury/512to511 and no-tick/hold boundaries
remain. No pool/grants/savev9 change. Explicit gear/protection/position/attack/phase/
timer/velocity/grab fixtures are not acquisition/ordinary encounter/routes/console.
Thrown release/stone grabs/other producers/death/fade/full sharing and Alucard remain
incomplete.

Selected [surviving ice-hit effects](docs/player-ice-hit.md) now owns actual
factory46/child33/twenty-four G4 rows,96 birth RNG and sound69D/61A. Preserve private
point pointers/SDK sqrt-atan2-trig, live player high-word overwrite/captured fractions,
status10000 versus step, airborne wall/substep5 and supplied physical-death16 delay.
Preserve same-update random shard delays, signed byte motion/wrap, inverted color
underflow, hidden-row updates and64-update retirement. No invented fade/artwork/pool.
Preserve player-lock entity-only shoves, callback/cull before camera, per-shard
flag2 versus room100 and no-tick holds. Selected original room-entry cleanup
retains20000 shells and releases detached child33/actual rows; other cleanup owners
remain unfinished. Freeze/menu/draw/release holds, allocation before RNG and late
sibling/runtime/row/sound/RNG rollback remain required. Supplied raw-kind/protection/
wall/status/death fixtures are not ordinary ice producers/acquisition/routes/console.
No grants/savev9 change; remaining damage/death/fade/gear/familiars/full sharing and
Alucard/castle remain incomplete.

Selected [surviving thunder-hit effects](docs/player-thunder-hit.md) now owns actual
factory45/four parameter0..3 children30, six GT4 rows each, sound69D/665 and
Blink46/eight rows. Preserve actual slot order, allocation-before11 birth RNG,
44-call full birth/eight sounds, six-row copy history/UV/CLUT/phase priority and
captured fractions versus live pre-camera player motion. Preserve private SDK
sine/cosine, signed wrap/shifts, airborne/stone/wall gates, phase/fade RNG order,
terminal decrement before work and no invented RGB fade. Blink7001 holds on state10;
its profile's7000 color divisor is not a geometry marker. Preserve optional US
Blink retry, actual leases/retirement, callback/cull before camera, flag2 versus
room100, frozen/menu/draw/release holds, no-tick rebases and late sibling/RNG/row/
sound rollback. Native wall arcs retire during hurt, so native menus and Spirit
are after arc retirement; active frozen holds are authored evidence. Supplied
raw-kind/protection/wall/high-parameter/transform/clock/state fixtures are not
ordinary thunder producers/acquisition/routes/console. No pool/grants/savev9 change;
ice/death/fade/gear/familiars/sharing and Alucard remain incomplete.

Selected [surviving fire-hit effects](docs/player-fire-hit.md) now owns actual
sound690/Blink45/eight rows, BP17/high1 and delayed BP18/low child16 streams, plus
real BP4/profile4 parent-bound eleven-row descendants. Preserve actual state10
factory cancellation, captured fractions/facing versus live pre-camera high words/
hitbox/VX,3 birth RNG/no birth motion, private finite animation after callback/cull
and terminal next-callback motion/retirement. Scale-disabled bank5 words still
wrap; use actual bank2/5/SDK/original UV/raw blending, no invented fade. Preserve
real parent slot/generation and VX read after row allocation, not live player VX.
Profile4 retains VX but moves Y only, has3 RNG calls per tile, signed delays,
lifetime31 double decrement/no RGB fade and actual sentinel retirement. Preserve
frozen/menu/draw/release holds, no-tick rebases, post-camera work, exhaustion before
RNG and late particle/parent/runtime rollback. Native Spirit checks occur after
fire retirement; active frozen holds are authored. Raw-kind/protection/wall/image/
parent/high2 fixtures are not ordinary fire producers/acquisition/routes/console;
alternate10000 and high2 have no selected native producer. No pool/grants/savev9
change; thunder/ice/death/fade/gear/familiars/sharing and Alucard remain incomplete.

Selected [surviving dark-hit effects](docs/player-dark-hit.md) now owns actual
113/child54/rowless private bank14 sprites and Blink62/eight rows. Preserve actual
origin7/state10 factory cancellation,96 upper bound/four-update schedule, captured
fractions/facing, allocation-before3 RNG and one whole-word shared blend phase.
Retain that actual DRA global after session bootstrap without savev9 fields.
Preserve no birth motion, callback/cull before private animation, terminal next-
callback retirement, SDK scale/original mirrored UVs and raw texture flags33:
decreasing opacity does not enable a fade. Preserve frozen/menu/draw/release holds,
no-tick rebases, post-camera translation and late sibling/global/RNG rollback.
Nine bounded hurt intents do not expand the actual eight-root pool. Raw-kind/
protection/wall/checkpoint/image fixtures are not ordinary dark producers,
acquisition/routes/console evidence; active frozen children are authored evidence,
separate from native Spirit-after-retirement. No pool/grants/savev9 change; fire/
thunder/ice/death/fade/gear/familiars/sharing and Alucard remain incomplete.

Selected [surviving holy-hit effects](docs/player-holy-hit.md) now owns actual
119/child47/sixteen GT4 rows, one shared DRA point array and Blink68/eight rows.
Preserve raw hitbox offsets, allocation-before64 RNG calls, sibling overwrite/slot
order, delay/UV/final-Y semantics and status10000 retirement after tiles finish.
Use pre-camera feedback, callback/cull before the camera pass, flag2 versus room
flag100 packets, entity-only actual stage shoves and no ordinary movement following.
Shared points never acquire camera translation. Preserve frozen flags, no-tick
rebases, late rollback and offscreen retirement after real birth RNG. Native menus
are checked after hit retirement; do not fake the host's pause gate or an active
holy/Spirit combination. Raw/kind/protection/wall/image fixtures are not ordinary
holy producers/acquisition/routes/console evidence. No pool/grants/savev9 change;
remaining elemental/death/fade/equipment/familiars/sharing and Alucard stay incomplete.

Selected [cut-hit effects](docs/player-cut-effects.md) now owns actual cut Blink42,
factory20/profile5, shared13/eight-row leases and ordered sound62E. Preserve earlier
ailment/elemental priority, captured heavy-hit fractions/facing, original vector
birth3calls per particle after allocation, retained actual AluInit vectors/64-call
initialization, lifetime47/sentinel/damping/halving/thickness and death6 behavior.
Preserve Blink's real allocation retry, skid-first root exhaustion, no unrelated
replacement/RNG on skipped particles, frozen particle holds versus Blink updates,
menu/draw/release holds, no-tick rebases and late rollback. Explicit Library arrival,
Dhuron positioning/later protection and image/rotation fixtures are not acquisition,
uninterrupted route or console evidence. Alternate10000 cut and other elemental/
stone/death/fade/equipment/familiar/shared gates remain open. No pool/grants/savev9
change; Alucard/castle remain incomplete.

Selected [poison/curse effects](docs/player-ailment-effects.md) now owns actual
new ailment Blink16/17 and shared text37 parameters1/2. Preserve curse text-before-
Blink versus poison Blink-before-text, earlier old-ailment/motion roots, actual
18-row wait/lease, private GT4 and sixteen grey TILEs. Preserve packed target120
with original half-width fraction, sine cap order,96 hold, tink6AF once, final
retirement Y increment, tile camera-flag loss and real offscreen retirement.
Ordinary camera shifts only tile flag2; room entry shifts the flag100 GT4 once,
never text extension coordinates/target. Preserve menu/draw holds, actual frozen
effect advances/release hold, late rollback and optional exhaustion. Library
arrival/seed spawn/actor positioning/later protection fixtures are not acquisition,
uninterrupted route or console evidence. No pool/grants/savev9 change; remaining
elemental/stone/death/fade/equipment/familiars/sharing and Alucard stay incomplete.

Selected [hurt motion/palettes](docs/player-hurt-motion.md) now owns actual kind0/1
promotion, selected2/3/4/5/7/16..18 births/recovery, timer8 and shared wall/ceiling/
slope/skid effects. Preserve pre-DEF power50 versus actual loss2 promotion, heavy
one-fraction Y shift, retained facing on upside-down interruption, direct2F cursor,
signed wall48 expiry, ceiling24/rebound8, stage18/24/56 exceptions, original root
order/fractions and optional exhaustion. Preserve real SDK pivots, shake2/3, wall644
before secondary/death voice, secondary Talisman/HP/number/Cloak/Fury priority and
no-tick rebases/holds. Actual effect-word palette/timer priority is separate from
unowned elemental children; selected poison/curse text/Blinks now have actual owners. Raw-kind/poison/protection
fixtures are not ordinary producer/acquisition/route/console evidence. No pool,
default-grant or savev9 change. Remaining damage/stone/grab/death/fade/equipment/
familiars/sharing and Alucard/castle remain incomplete. Preserve the explicit
unsupported US status20000 smoke birth; never guess or bypass its parameter.

Selected [player timer ownership](docs/player-timers.md) now unifies the actual16
word counters, separate from HUD stat buffs. Preserve raw signed-word wrap/signed
minimum, per-index actions/decrement/expiry, poison4/14/15 twelve-to-eleven birth,
prior ailment masks,13-to14 sixteen-to-fifteen, timer6 fall gate and actual SDK colors.
Named counters alias one bank; Dark snapshot copies11, weapon9/status400 and use10/
status800 retain actual producers. Preserve both13/14 protection gates and longer
hurt/landing protection. Normal stone clears actual0/1/2; Axe conversion retains
ailments. Timer15 counts even unbound, but only its real leased owner enables
presentation. Freeze/release/menu/map/draw/dead hold the pass; actual shared producers
may still write their words. Preserve failure rollback, no-tick rebases, post-bootstrap
ailment stage carry, actual native Dark palette-job clock and v9 Continue transients.
No pool/grants/save change; timer storage alone supplies no impact/Blink/consumable
producer. Selected hurt now owns its actual impact8 producer. Seed/
protection/arrival/save-room fixtures are not acquisition/routes/console evidence.
Milestone1 remains32/34 plus common/cross-owner gates; Alucard/castle are incomplete.

Selected [Axe Lord Armor](docs/player-axe-armor.md) now owns actual body25/virtual216/
weapon45, states40/41/42/43/50, shared slot16 child50/Spell11/group1/native combat,
rowless shadow49 and private SDK multipart rendering. Preserve owned selection,
32-input hold versus silent bootstrap, overlay-word lifetime, priorities/signed
motion, kind17/18 sides versus converted kind1, zero-stone hurt/number, retained
ailments/healing/Cloak and real normal-body death. Preserve angle/byte wrap,
pivot20/512/896/48-recovery, raw-mode2/script palette holds, actual root exhaustion
before RNG, captured facing/child fractions, slot/group cooldown and row retirement.
Rendering restores actual hand banks; original Blink/outline Axe exclusions remain.
Freeze/menu/map/draw holds and no-tick rebases remain; no pool/grants/savev9 change.
Owned-item/position/protection/mode/arrival/save-room fixtures are not acquisition/
route/console evidence. Current dispatch is32/34;17/39 and common/cross-owner gates
keep milestone1 open. Earlier batch counts are historical; the checklist is current.

Selected [neutral hurt feedback](docs/player-neutral-hurt.md) now owns accepted
queued response3's neutral Blink58 and original red timer2six, plus selected
hurt-exit protection12/timer4twelve/afterimage4. Preserve pre-reaction fractional
origins, prior birth palette/redfive/normal expiry, full-parameter duplicate rules,
real factory44/entity11/eight-row US retry/retirement and afterimage resets.
Existing poison/curse allow neutral birth but suppress exit color/palette/timer15
restoration; death/stone/grab/ongoing hurt/scripts hold that exit helper. Preserve
longer protection, actual private SDK colors, successful-player commit/rollback,
frozen effect updates, release/menu/map/draw holds and no-tick rebases. Number and
Blink exhaustion stay independent; no pool/RNG/sound/grants/savesv9 change.
This does not implement damage-kind promotion/motion or elemental reactions;
milestone1/Alucard remain incomplete at26/34 states. Explicit protection/contact/
pose/item/arrival fixtures are not acquisition/route/console evidence.

Selected [queued contact numbers](docs/player-contact-numbers.md) now own actual
response3/4/7/8 type0 producers through shared entity19/slots56..63/twenty rows.
Preserve original uncapped requested points (through39996), unsigned-word/four-low-digit
display, pre-reaction fractions, zero-stone blue and lethal overkill red.
Same-hit stone escape retains its accepted response8 number. Prior green healing
stays first; immune/guard keep existing producers, Talisman/water add no number.
Preserve successful-player commit, failure rollback, no-tick rebases, real row
leases/retirement, freeze effect updates and release/menu/map/draw holds. Full
stage/Continue clears transients; game over holds the scene. No pool/RNG/grants/
ordinary-savev9 changes. Controlled item/raw-contact/protection fixtures are not
acquisition/Medusa/ordinary lethal route/console evidence. Remaining hurt/kinds/
elemental/interrupt death/stone/grab/impact/MP producers/sharing and milestone1/
Alucard remain incomplete at26/34 states.

Selected [Ballroom Mask](docs/player-ballroom-mask.md) now applies actual head28's
conditional raw-contact reduction after weakness/resistance/US stone immunity/
immunity/absorption and before defense/poison/new stone/guard/hurt selection.
Preserve attack-minus-truncated-fifth on effectsF980, raw strongest/first-equal
queue priority, actual one-head ownership, existing-stone/environment/water
fractions, Talisman's later roll and reduced surviving Blood Cloak/Fury boundaries.
No new RNG/effect/pool/grants/savev9 fields. Bind actual equipment before consuming
queued contacts, including menu changes; preserve healing, rollback, freeze/
release/menu/map/draw holds. Explicit arrival/item/position/protection fixtures are
not acquisition/routes/console evidence. Remaining hurt/elemental/interrupt death/
stone/gear/familiars/sharing and milestone1/Alucard remain incomplete at26/34 states.

Selected [Talisman dodge](docs/player-contact-talisman.md) now owns response2:
actual accessory83 count, derived LCK, one inclusive roll before guard/hurt and
actual shared factory115/entity46 pair/Blink58. Preserve earlier immunity/absorb/
Cat-eye/stone/water priority, silent no-HP/recoil/ailment/number/cloak/Fury result,
pre-controller fractional request and successful-player commit. Reuse actual
high_jump_timer/timer3 recovery, not a second timer. Preserve live birth high words/
facing, retained fractions, no birth movement, odd gameplay-clock signed motion,
red/gray fade and Blink26 phase18 retirement, private9F/SDK/slot12 resident upload,
real eight-row leases, optional visual exhaustion and shared afterimage resets.
Frozen effects use held gameplay-clock parity; release/menu/map/draw holds and
no-tick rebases remain. No pool/default grants/resources/savev9 changes. Owned
selection/later protection fixtures are not acquisition/route/console evidence.
Remaining hurt/elemental/interrupt death/stone/equipment/sharing and milestone1/
Alucard remain incomplete at26/34 states.

Selected [high-defense guard](docs/player-contact-guard.md) now owns surviving
response1: signed nonpositive damage/DEF100, one HP/no recoil, separate voice6E7 before later control sounds, red
number1, protection12 and timer3six. Preserve immunity/absorption/stone/water
priority, poison/curse-bearing exclusion, existing ailments, prior deferred healing,
Blood Cloak/Fury and pre-controller fractional origins. Use actual high_jump_timer
alias in gravity.timer, prior status before ailment decrement, timer2/3/4 order,
expiry normal8100 and same-pass 12-to11 protection/afterimage/color timing.
Owned God'sGarb24/Nauglamir87 additive stats support actual selections; retained
real Fury buff supplies the native controlled DEF102 path. Type0 red/blue numbers
share real entity19/twenty rows with green healing. Keep optional exhaustion,
freeze/release/menu/map/draw/no-tick and rollback boundaries. No pool/grants/savev9
changes. CON/equipment/protection fixtures are not acquisition/routes/console
proof. Remaining hurt/elemental and interrupted death/stone/equipment,
sharing and milestone1/Alucard remain incomplete at26/34 states.

Selected [contact immunity](docs/player-contact-immunity.md) now owns actual response0,
factory47/entity37 parameter0 text, Blink43 then51, type0 zero number and protection12.
Owned Coral44 uses private curse immunity; preserve US stone-element removal order,
immunity before absorption/Cat-eye, prior deferred healing, existing ailments,
pre-controller fractional sources and commit after player success. Preserve actual
current/prior-frame ground wait, delayed eighteen-row text lease, hidden spare rows,
private metadata/SDK sine, signed wrap, fifty-six display updates and real retirement.
Zero19 uses the actual twenty-row owner with original blue palette/trails; green
healing remains separate. Optional exhaustion omits visuals; US Blink retries remain.
Initialized effects advance on frozen updates while protection holds; release-only/
menu/map/draw and no-tick rebases retain their boundaries. No new pool/grants/savesv9.
Equipment/arrival/Ectoplasm-position/post-contact-protection fixtures are not route/
acquisition/console evidence. Guard/Talisman/hurt/elemental death/stone/other text,
equipment/shared contention and milestone1/Alucard remain incomplete at26/34 states.

Selected [Alucard sword command/recovery](docs/player-sword-warp.md) now owns actual
QCF/virtual197/five-MP, state15, shared root54/slot16 and next-pass strike62,
private weapon43 images/animation/combat, outlines20/21 and sixteen-update recovery.
Preserve pre-collision requests, real reservation outcomes, single strike RNG,
allocation-before-RNG, least-used counter increment before parent enemyId0 copy,
retained group0 cooldown, whole fixed/word wrap, hidden810D and normal8100 palettes,
pose5/14/19 gates and no birth motion. Retain virtual equipment/artwork through
supported owned hand changes; restore the selected native overlay after drawing.
Menu/map/freeze/draw holds and no-tick rebases remain; failed player updates undo
actual reservations, failed shared ticks retain runtime/character RNG. No new pool,
grants or savesv9 changes. Spawned/positioned targets, owned-equipment selections
and malformed-image fixtures are not acquisition/route/console evidence. Osafune,
ordinary slash sparkles, other weapon specials/full overlay contention and the
remaining eight unhandled states/control branches keep milestone1 incomplete.

Selected [contact absorption](docs/player-contact-absorption.md) now owns original
immediate response5, silent Blink44 then48, green number and timer14 protection12.
Owned circlets38..42 use actual private attributes; Cat-eye43 converts kind7 only
after ordinary absorption/immunity. Preserve weakness/resistance and US stone
immunity order, minimum-one, fullHP/stone effects, existing ailment timers,
pre-controller fractions and request commit after player success. Deferred healing
still runs first and retains its independent number/sound; absorption gives no
Blood Cloak/Fury work. Keep full unsigned-word/four-low-digit numbers and optional
exhaustion. Freeze holds HP/protection while flag10000 effects advance; release-only/
menu/map/draws hold effects. Preserve no-tick rebases. No new pool,
grants or savesv9 change. Equipment/HP/arrival/seed-position/protection/kind7 fixtures
are not acquisition/route/console evidence. The linked source-backed damage
inventory retains incomplete immunity/guard/Talisman/hurt/elemental death/stone
branches; full equipment/familiars/sharing/Alucard/castle remain unfinished.

Selected [deferred healing numbers](docs/player-healing-numbers.md) now own direct
entity19/first-free slots56..63, twenty real shared rows, private digit GT4s and
sixteen trail tiles for both healing kinds. Preserve uncapped requested amounts,
full-HP/stone numbers, pre-controller fractions, twelve phases/birth+108 updates,
private SDK math, signed wrap, descending trail births and optional exhaustion.
Initialized numbers advance on frozen effect frames; release-only/menu/map/draws
hold. Preserve no-tick rebases, offscreen keep-alive and actual row retirement.
No second pool, grants or savesv9 change. Explicit equipment/HP/MP/protection/pose
fixtures are not acquisition/route/console evidence. Other number producers and
damage/equipment/familiars/sharing/Alucard/castle parity remain incomplete.

Selected [scripted input/reaction](docs/player-script-control.md) now binds Death's
actual mode1 request to state48, normal reset, timer2/red palette, real BP0/Blink58
and animation/WHAT/recovery. Preserve the single stage/Bat/Wolf input counter,
raw mode2 hold, US physical-input rebase, timer4's private SDK colors/gameplay clock,
timer15 boundary, optional shared exhaustion, pending US retries and failed-update
rollback. Mist interruption destroys its actual generation without fake readiness
or palette-job resets. Dead/frozen/menu/map/draw paths hold counters and never
revive. Explicit room/relic/pose/palette/lethal/protection fixtures are not route/
acquisition/console evidence. No default grants or savesv9 changes; milestone1 remains open.

Selected [normal control corrections](docs/player-control-audit.md) preserve original
timer6 ledge expiry, heavy-Jump versus attack landing, fast landing momentum,
ordered standing/crouch hand cancels, Basilard holds, prior-child shield state,
Jump's early return and low-speed Bat/return eight-pass collision. Preserve the US
first-physical-frame tap rebase after selected simulated input, failure rollback,
no-tick room rebases and menu/map/draw holds. Held shield startup is not a subweapon
tap. The source-backed inventory has 34 original handled states versus 26 native
dispatches; all remaining equipment/familiar/script/damage/death dependencies must
close before milestone 1 is complete. Explicit equipment/pose/protection fixtures
are not acquisition/route/console evidence. No default grants or savesv9 changes.

Selected [Blood Cloak](docs/player-blood-cloak.md) now owns surviving selected queued
contact refunds, actual shared factory99/child51, private sprite rendering and pickup
sound. Preserve pre-add full-heart suppression, zero-loss stone AddHearts(0), lethal/
immune/absorb/ordinary-water exclusion, pre-reaction fractional origins and commit
after player success before session handoff. Optional exhaustion omits only the
visual. Preserve128 motion updates, private SDK cosine, signed wrap, clock-modulo4
sway, slot/engine-clock Blink, freeze/menu/draw holds, no-tick rebases and real
offscreen retirement. No new pool/grants/savesv9 change. Explicit cape/pose/clock/
protection fixtures are not acquisition/route/console evidence. Remaining damage/
equipment/familiars/shared contention and Alucard/castle parity remain unfinished.
The [Alucard milestone checklist](docs/alucard-status.md) consolidates current
availability and completion gates; individual ownership documents retain their
evidence boundaries. Older batch notes can contain superseded availability lists.

Selected [HUD resources/Healing Mail](docs/hud-healing-resources.md) now owns actual
walking128/counter reset, kind2 deferred healing and actual gameplay-clock MP.
Preserve status-based walking, Mail overwrite versus blood first-pending priority,
silent Blink48 versus kind1 sound/Blink48+44, frozen/dead pending holds and no
revival. HUD runs ordinary/frozen/release, holds menu/map/draw/GameOver; Mail resets
on full stage/Continue, unlike retained global buffs. MP has no HP gate, two
Pendants still add one, and fresh actual MP binds before next player control.
Do not substitute saved playtime or a cooldown for actual gameplay phases/status.
Other equipment and remaining number producers stay incomplete; no fake effects/pool.
Explicit grants/health/protection/clock/pose/arrival fixtures are not acquisition,
route or console evidence. Savesv9/default grants/Alucard incompleteness unchanged.

Selected [stat buffs/Fury Plate](docs/stat-buff-ownership.md) now owns actual
sixteen-word DRA counters, derived stat masks, pre-damage attributes and surviving
Fury refreshes. Preserve all16 HUD decrements (including the switch default),
512-to511 birth, longer buffs, ordinary/frozen/release clocks, menu/map/draw holds,
new/Continue reset versus connected-stage retention after bootstrap and request
commit only after player success. No second pool or ordinary-save fields. Preserve
pre-cap bonuses, signed-word attack/DEF, computed original SDK root normalization,
US stone immunity and no stale defense after expiry. Fury and selected owned
medicines now supply live producers; masks/Give alone are not remaining consumable/
other-equipment availability. Explicit
equipment/pose/protection/clock/arrival fixtures and authored metadata copies are
not acquisition/route/console evidence. Default grants/savesv9 and Alucard/castle
incompleteness remain unchanged.

Selected [ordinary water damage/contact ownership](docs/player-water-contact.md)
now queues actual raw stage contacts after combat and consumes them after healing/
timers before player control. Preserve strongest/first-equal priority, high-only
water replacement before protection/grabs, one reaction birth/animation, actual
Holy ownership, wrapping gameplay clock, shared Blink4E/sounds and DRA voice timer.
Menus/freeze/draws hold HP/counters; session handoffs restore clock/counter after
bootstrap without serializing them. Dead states retain input/timers/healing without
revival. Failed updates retain queued contacts/HP/RNG/counter; native capture
rollback remains required. Owned Holy can toggle; no default grants/savesv9 change.
Fury now has its actual stat-buff owner. US wet-smoke remains explicitly unsupported,
never guessed or silently enabled. Supplied pose/protection/clock fixtures and the actual app's
ordinary70HP water-position fixture are not route/acquisition/console evidence.
Full damage/equipment/fade/timing, Alucard and castle remain incomplete.

Selected [native water death](docs/player-water-death.md) now consumes an actual
lethal result through state13/C1, normal reset, shared birth effects, resident
texture capture and64 committed decay/uploads to real game over. Preserve actual
generation/callback ownership, retained pivots, birth rotation-flag clear, original
color/velocity/pose boundaries, no artwork reupload after capture, frozen/repeated
draw clocks and real texture/factory rollback on late failure. Dead healing stays
pending without revival. Supplied lethal-result/HP1/pose and protected form-entry
fixtures are not ordinary water HP or route/console evidence. Selected ordinary
water/queued damage now runs; global-fade recovery remains unwired. No replacement timer or fake readiness.
Ordinary starts/grants/savesv9 and Alucard/castle incompleteness remain unchanged.

The [owned death-image APIs](docs/player-death-capture.md) now copy actual resident
display-texture pixels and reconstruct original packed-nibble decay/uploads.
Preserve private rectangle bounds, full5120-byte copies, four40-nibble groups,
cursor+35 byte wrap,64-update completion, immutable source art and unrelated VRAM.
Commit copied cursors only after actual upload succeeds. Requests are not completed
player color/velocity/substate/game-over transitions. Selected native water death
now owns those transitions; earlier explicit decay/draw fixtures are not dying-player/route/console
evidence. Physical death is not a water-death substitute. Savesv9/default grants
and Alucard/castle incompleteness remain unchanged.

Selected [death particles](docs/player-death-particles.md) now bind original
AluInit vectors to shared profile6 factories/rows/rendering. Initialize32 vectors
with64 actual selected-character RNG calls once after full new/Continue restoration;
room entry/menus/draws retain them. Preserve three-call births, whole-word wrap,
retained vectors, signed damping/halving, single lifetime decrement, mode/thickness
thresholds, sentinel and optional exhaustion before RNG. No substitute vectors or
particle pool. This corrects initialization RNG consumption without changing HP/MP/
equipment/relic defaults or savesv9. Native supplied death-factory/protected/pose
fixtures are not actual dying-player, acquisition, route or console evidence.
Selected native water death now supplies birth/capture/common death. Ordinary
water HP/contact production and Alucard/castle remain incomplete.

Selected [water Blink effects](docs/water-blink-effects.md) now run parameters4E/59
through actual shared factories/eight-row leases/rendering and native waist/Holy/
animation snapshots. Preserve pain7004 and death7005's animationC0 condition,
duplicate full parameters, pending US retries, original fan/colors/rotation,
afterimage resets and no-tick rebases. Explicit native requests/relic/pose fixtures
are not live water HP/death, ordinary acquisition or route/console evidence.
Selected native water death supplies its producer/capture/common death; selected
ordinary water HP/contact production now runs. Savesv9/default grants and
Alucard/castle incompleteness remain unchanged.

The [water damage/death components](docs/water-damage-death.md) now reconstruct
original waist injection, priority/protection gates, kind6 HP/Fury work, pain timer
and state13 color/velocity/helper/capture transitions. Preserve high-only contact
replacement before grab/invincibility, absolute modulo10, repeated Blink4E and
voice refresh, lethal response6, birth rotation-flag clear, signed velocity wrap,
strict reset and fade preemption. Stone/other contacts delegate to real damage;
helper/factory/capture requests are not completed native operations. Selected native
HP/contact/capture/common death now run; actual remaining equipment/effects owners
are required before extending their scope.
Ordinary physical death is not a substitute. Savesv9/default grants stay unchanged;
native water damage/death and Alucard/castle/console parity remain incomplete.

Selected [normal water bubbles](docs/normal-water-bubbles.md) now use real shared
factory4/profile13 children and original standing/Crouch0 source/counter gates.
Preserve strict48/24 depths, shield-specific countdown, expiration-only decrement,
full fractions/facing, optional failure cooldown and bubble-before-walking-puff
order. Both requests may coexist. The shared DRA counter survives room/session
replacement after bootstrap without an extra tick; never put it in v9 saves.
Freeze clears requests, not the counter. Standing native checks use a fixed-floor
pose fixture; crouch uses ordinary collision. The existing unsupported US wet-smoke
birth guard remains; do not guess its unwritten value. Water damage/death and
Alucard/castle/console parity remain incomplete; default grants stay unchanged.

Selected [native player water feedback](docs/player-water.md) now binds actual
prior-stage feet/waist/head samples, selected normal gravity/drag/status,
Mist/Bat/Wolf waist gates, Wolf swimming/falling and shared particle water input.
Preserve thresholds40/41 and Wolf12/13, signed wrapping/truncation, capture before
drag, post-collision restore, no-tick room clears and actual bubble/factory rows.
Original entity byte49 is NOT the damage-invincibility timer; selected producers
leave it zero, collision clears it only within its original status boundary.
Water damage/death and further state/equipment/stage/route/console parity remain
unfinished. No default grants or savesv9 change; Alucard/castle remain incomplete.

Selected [native Entrance water actors](docs/native-water-actors.md) now commit
actual controller plans, auxiliary slots224..255, shared primitive leases,
original GT4/tile rendering, sound requests, currents and retained three depths.
Preserve pre-current sampling, zero child fractions, separate stage/libc RNG,
default drop priority, contiguous33-row sentinel, no-tick camera translation,
optional exhaustion and late-birth row rollback. Room entry clears water depths.
Selected player drag/forms/swimming/bubbles now bind those real depths; selected ordinary water
damage now runs. Never substitute tile contacts for real depth words.
Savesv9/default grants and Alucard/castle/console
incompleteness remain unchanged.

The [ordinary water-effect components](docs/water-effects.md) reconstruct main/
surfacing/side GT4s and small/32-particle drops. Preserve real2/4/1/33-row leases,
allocation-before-RNG, separate stage/libc streams, whole-word child origins,
birth-motion distinctions, cached world surfaces, packed fractions and sentinel.
Camera shifts body/packets once; draws never tick. Sound/drop requests are not
completed playback/factories. Selected native Entrance owners now supply stage
actor/primitive/render ownership and selected player consumers; selected ordinary water damage now runs.
No new pool/default grants/saves change. Alucard/castle/console
parity remain incomplete.

The [ordinary water-controller component](docs/water-controller.md) reconstructs
birth/cache timing, ordered splash/surfacing plans, actual auxiliary occupancy,
cooldown and signed currents. Preserve original entry-variant overwrite, whole-word
Y tests, prior exit row/origins, zero child fractions and pre-current sampling.
Plans are not completed factories: validate/commit every real slot224..255 before
controller/depth/current feedback. No second pool or fake surface cooldown is
allowed. Selected native Entrance now supplies placed/effect owners and selected
player consumers; selected ordinary water damage now runs. Ordinary starts/savesv9 stay unchanged.
Alucard/castle/console parity remain incomplete.

The [water-volume query](docs/water-volume-query.md) reconstructs original private
volume/slope sampling and three distinct form-dependent depths. Preserve word wrap,
strict right/bottom edges, first match, contact at zero depth and crouch/form
precedence. Unused no-contact scratch initializes safely and confers no selected
row/readiness. Selected native Entrance now supplies placed controllers/currents/
splashes and selected player feedback; selected ordinary water damage now runs. No controls/default grants/
saves change. Do not substitute a tile
boolean or invent successful auxiliary allocation. Alucard/castle/console parity
remain incomplete.

Selected [playable Soul Steal](docs/player-soul-steal.md) now owns original
command/50MP/learning5, actual state37/animationDA/pose7 requests, flash12/outlines22,
shared factory112/child53/slot47, global hit-state82 and native damage/orb healing.
Preserve captured pre-collision origins, exact-cost casts, shield removal, no scene
freeze, shared group7..10 cooldown across births and optional failure before RNG.
Stage contacts bypass AABB through original bit80, retain active-screen/mask/slot
rules and spawn real orbs BEFORE damage/immunity. Never replace Echo or unrelated
actors on exhaustion. Menus/draws never tick owners; rebases add no updates.
v9 saves/default20MP starts stay unchanged. Protected MP50/HP50 explicit fixtures
are not ordinary acquisition/route/console parity; Alucard/castle remain incomplete.

Selected [Soul Steal orbs](docs/soul-steal-orbs.md) now own actual common actor7,
one real shared stage row, original private homing/animation/opaque GT4 and deferred
healing. Preserve allocation-before-RNG, original stage Random, age16 contact,
first-pending-heal precedence, eight HP without Bloodstone scaling and no-tick
rebases. The native dispatcher records contact through invincibility and retires
real rows. Selected [playable Soul Steal](docs/player-soul-steal.md) supplies the
actual command/global attack owner; this component alone establishes no casting
readiness. Savesv9/default grants stay unchanged; Alucard/castle/console and full
shared contention remain incomplete.

Selected [playable Hellfire](docs/player-hellfire.md) now owns ordinary commands,
MP15/learning, real same-frame root reservation, original hidden steering/recovery,
actual private gradient uploads, sprite/G4 rendering and native enemy damage.
Preserve feedback after stage commit, pre-collision factory origins, live-Up choice,
signal3-only Down cancel, timer12/circle timing, original exhaustion fallback and
rollback of actual reservations after failed player updates. Finishing restores
real poison/curse Blink16/17 with actual timers; no invented palette reset. Bind
the current session callback after replacement; draws never tick. Ordinary starts
and savesv9 stay unchanged. Protected explicit checks are not ordinary-route/full
Hellfire/shared-contention/Alucard/castle/console parity.

The [shared Hellfire runtime](docs/shared-hellfire-runtime.md) owns actual
factories/children24..29, one-row leases, private animation, spell-two attacks/groups
and selected native damage. Preserve next-pass descendants, real reservation
outcomes, root/flash priority448, pre-movement trail origins, separate trigger/birth
RNG, raw groups/slot31 survival and no-tick rebases. Upload requests remain intents
until the native owner commits actual pixels. Selected playable Hellfire now
supplies its player/MP/learning/gradient/palette/render owners. Full sharing,
Alucard/castle/console parity remain incomplete; supplied-cast stage checks are
separate evidence from ordinary command checks.

The [Hellfire casting controller](docs/hellfire-casting.md) reconstructs original
phase0..4 actions. Preserve failure-normal then animation1/substate increment,
hidden steering floor/edge tests, signal2 stop, signal3-only Down cancel, timer12
before circle37 and ordered finishing/status-flash calls, including double normal
on failure+FFFF. Selected native ownership now consumes these actions; this
interface alone enables no controls or full-Hellfire/Alucard/console parity.

The [Hellfire supporting effects](docs/hellfire-effects.md) reconstruct circle28,
flash29, selected trail24 and rising particle profile10. Preserve real leases,
post-increment flash retirement, failureFFFF without successful signal reset,
live high words/fractions, trail birth-only RNG and profile10 no-RNG/double lifetime.
Shared particle culling uses actual KEEP04000000 after initialization. Selected
native Hellfire now consumes these effects. Full sharing/parity remain incomplete.

The [Hellfire fireball components](docs/hellfire-fireballs.md) reconstruct normal
spread/byte-hit retirement and big pre-animation-gated acceleration/piercing/rotation.
Preserve private vectors, no birth movement, twentieth curved Y update, retained
velocity, raw speed16, draw flags/facing and signed fixed/word wrap. Big motion RNG
is separate from spell initialization and survives optional factory exhaustion.
Requests confer no attack/factory readiness; selected native owners now consume
them. Saves/default grants and scoped comparison claims remain unchanged.

The [Hellfire beam component](docs/hellfire-beam.md) reconstructs original one-row
failure/signalFFFF, phases/quad, palette/sound/gradient and live-Up requests.
Preserve high-word fractions, signed dimensions/timers, width24/negative hidden
width, post-phase color work and exact signal timing. The selected native owner
now supplies real rows/gradients/palettes/factories/combat/rendering. Cancellation
uses actual player recovery/status-flash restoration, without an invented palette
reset. Component comparisons alone prove no complete ability, Alucard or console parity.

Selected [playable Summon/Tetra Spirit](docs/player-spirit-spells.md) now connects
normal commands/MP/learning, actual factories/rows/targets/combat, prior-frame63x63
capture, black portal/body/trail/Blink presentation and real scene/audio freeze.
Preserve ordinary birth,33 frozen updates in the selected private contexts, final
frozen retirement and separate release-only frame; physical input updates but
ordinary clocks/timers/resources/hits/menus/saves do not. Consume capture once
before drawing; acknowledge committed marks, retain raw target slots and carry
cursor across sessions without saving it. The shared capture cache advances on
actual effect updates, including freeze, and stays held through menus. Off-frame
capture sources fail explicitly. v9 saves/default grants remain unchanged.
Protected/positioned and in-memory transfer checks are not whole-route/console
parity; full OT/raster/audio/camera/contention and Alucard remain incomplete.

The [shared spirit runtime](docs/shared-spirit-runtime.md) now owns selected real
portal/spirit/Blink factories,9/16-row leases, animation, private spell attacks,
actual target snapshots/atomic marks and native enemy damage/groups. Preserve the
snapshotted frozen gate, next-frame descendants, pre-animation birth trails, raw
retained target slots, camera-after-steering, global flash tail and Wolf slot31
survival/group counts. New marks need acknowledgement after successful stage commit.
Capture intents alone are not completed copies; selected native owners now fulfil
them through the playable integration above. Portal-row exhaustion fails
explicitly before an indefinite original freeze; optional spirit exhaustion retires
before RNG. Saves stayv9/default grants unchanged. Full sharing/Alucard/castle/
console parity remain incomplete.

The [spell sound-script service](docs/spell-sound-freeze.md) now accepts queued
original pause/resume commands0xE/0xF, preserves mode5 and continuing voices/music,
and retains four real script channels with separate cursor/allocation lifetimes.
Preserve strict-before-equal priority, ringing leases, unreserved restore overwrite,
dropped ordinary copies, retained blocked fourth copy and original fourth-channel
key-off12..15 versus playback14..17. Selected native spirit freeze now consumes
these commands; full SDK/SEQ/audio timing and console parity remain incomplete. Other spell controls still require
their actual capture/factory/combat/engine owners.

The [selected frozen-stage dispatcher](docs/stage-spell-freeze.md) now initializes
real step-zero actors and updates only flag10000 initialized actors, including
private hit-palette countdown and actual behavior/hit clearing. Preserve original
slot order, queued contacts and real primitive retirement; no layout scroll/culling,
hit detection, stun/cooldown/control/game-clock advances belong in this path.
The US palette reference parser validates its exported instruction/data bounds.
Selected native spirit owners now connect this interface; the dispatcher alone
does not establish whole-scene/Alucard/castle/console completeness.

The [spirit casting/freeze foundation](docs/spirit-casting.md) now supplies original
Summon/Tetra cast states, timer12, crouch recovery and selected animation-only
freeze/release boundaries. The selected native context binds actual stage freeze/
capture/children/combat owners; standalone callers still default to disabled.
Preserve the ordinary birth frame,
frozen retirement frame and following release-only frame; never tick player timers
or replay one-shot events during freeze. Only actual attack substates run crouched
weapon recovery. Ordinary starts and v9 saves remain unchanged; do not claim live
spirit spells, Alucard, castle or console parity complete.

# Working in sotn-native

The [spirit spell components](docs/spirit-spells.md) reconstruct the original
portal, child flight/trails and128-slot target selector. Preserve real9/16-row
requests, pending capture/factory boundaries, the original freeze retained after
portal allocation failure, private adjacent birth UV bytes, signed-word/fixed-point
wrap, retained target-slot semantics and mark/cursor ordering without RNG. They
own no native scene freeze/capture/combat/animation or actual row leases and enable
no new spell controls by themselves. Selected native owners now connect them;
further controls still require actual ownership.
Stage/game/Alucard/shared-contention and console parity remain incomplete.

Selected [playable Dark Metamorphosis](docs/player-dark-metamorphosis.md) now owns
normal commands, original state32/animation/MP/learning, duration, six real outline
children, global flash, palette job23 and actual blood absorption. Preserve captured
facing, exact-cost casts, retained hand-recovery words, shield removal, original
odd-frame flash, INT duration, timer cancellation on actual form entry and no-tick
rebases. Glow shares real Mist palette-job capacity and continues through engine
pause without changing simulation; never reset its phase or mutate source colors.
Protected HP/position/equipment fixtures are not ordinary encounter/route parity.
Saves stay v9; no default grants. Other spells/equipment/familiars/water/shared
contention and Alucard remain incomplete.

Selected [enemy blood/deferred healing](docs/enemy-blood.md) now connects cutting
hits to actual common actors and twelve contiguous stage rows. Preserve distinct
stage Random, descending particle order, packed fractions, permanent absorption
cutoff, pending-heal precedence and Bloodstone8/16. The next successful player tick
applies HP once and requests real sound/two shared flash owners; stone suppresses
flashes and death is never revived. Selected Dark now supplies its casting/status
owner; earlier queued-heal/contact fixtures remain distinct evidence. Keep ordinary
starts/saves unchanged. Full effects/shared contention/Alucard parity remain incomplete.

Selected [shared native outlines](docs/shared-player-outlines.md) now consume real
factory children, full parameters, stage primitive leases, uploaded artwork and
actual afterimage resets. Preserve all six blueprint40 lifetimes, retained original
player pivots, no-tick rebases, unrelated owners and late-failure rollback. Native
Dark now supplies its spell producer; MP/curse and other spell producers remain unwired.
Unit body scale/normal palette are the supported owner context; other transform/
palette producers and Alucard remain incomplete. Saves/ordinary starts are unchanged.

The [player outline component](docs/player-outline.md) reconstructs entity31
profiles0..23 and original SDK packets. Preserve birth scales/pivots versus live
pose/facing, byte/word wrap, distinct profile15/16/18/19 paths, scale-before-rotation
and post-geometry afterimage reset. Selected [native ownership](docs/shared-player-outlines.md)
now runs; the component itself enables no spells. No substitute pool or readiness
is authorized. Ordinary starts/saves remain unchanged.

The [ordinary spell command component](docs/normal-spell-commands.md) now reconstructs
six original recognizers and copied MP requests. Preserve full-word/subset input
tests, signed-word timers, facing captures, relative priority, expiration-frame
casts, Soul Steal asymmetry and retained Sword Brothers notifications. Availability
requires an actual native spell owner; default zero enables no controls. Selected Dark/Summon/Tetra/Hellfire/Soul Steal now bind their actual owners; other spells
remain unwired. Do not claim complete spells
or Alucard from component instruction comparisons.

Selected [Alucard status reconstruction](docs/player-status.md) now shares supported
normal/form/ailment/use/invincibility masks with native snapshots and afterimages.
Preserve walking capture clear, crouch-two/seated/death80 distinctions, separate
use/invincibility bits and immutable snapshots. Physical hit/stone now freeze native
MP recovery; normal movement resumes its existing absolute-clock cadence. The
collision owner retains its selected form word. Full palette/helper-timing/water/
spell/equipment/familiar/shared-effect parity remains incomplete.

Selected [normal backdash/movement puffs](docs/player-backdash.md) now connect A /
Xbox Y, original standing phase/cancel gates, scrape/landing sounds and actual
optional smoke factories. Preserve held-versus-tapped recovery, animation lock,
pre-collision room-space origins and zero-blueprint request identity. Walk starts
use original profile five with low parameters 0..2 and reversed velocity indexing.
Shared allocation failure omits only optional effects; no new pool/RNG/MP is used.
Do not replace the original undefined US smoke birth branch with guessed state.
Normal status/water/spell/equipment/familiar/full-sharing parity remains incomplete.

Selected [native Alucard afterimages](docs/player-afterimages.md) now connect the
original six samples/three global displays, timer15/reset hooks, private ARC slots
and ordered color/texture rendering to the real common eight-row stage lease.
Preserve pre-collision capture/post-collision fade, previous full button words,
frame-load skips, private slot flips, display fractions and no-tick room rebases.
The unsigned engine clock advances through pause and survives session replacement;
simulation/display cursors pause and renders never tick. Later real BlinkWhite
requests clear the actual player displays. Shake enters projection once. Ordinary
starts grant no relics; saves remain v9. Full status/script/palette effects, GPU/
all-ability contention and Alucard/route/console parity remain incomplete.

Selected [Gravity Boots ceiling effects](docs/player-gravity-impact.md) now consume
one-update impact intents through actual shared factories/children/rows and shake3.
Preserve pre-flip facing, captured room-space X±3/Y-22 with fractions/word wrap,
no-tick room rebases, no double camera shift, optional allocation before RNG,
particle sentinel ownership and late-failure rollback. Existing jump MP/motion and
ordinary saves remain unchanged. Other impact producers/shared effects and Alucard
remain incomplete; private relic grants are not ordinary acquisition parity.

Selected [Wolf camera shake](docs/player-camera-shake.md) now consumes actual impact
requests and applies original presentation offsets. Preserve last-request-wins,
other-axis retention, look-ahead terminal sample discard, no-op identities 0/5,
paused/repeated-draw clocks and unshifted logical camera/collision. Wolf entity
projection applies shake before normal/capture packets; never add it twice at draw.
Menu/absolute packet flags retain target offsets while omitting shake. Patterns stay
private. Other shake producers, full primitive flags/shared effects and parity remain
incomplete; do not call Wolf or Alucard complete.

Selected [playable Wolf](docs/player-wolf.md) now binds native contexts/body
feedback, R/Xbox RT, bites/rush/charge combat, MP/learning, original rendering and
owned Soul/Power/Skill switches. Preserve real pending entry, once-per-clock body
feedback, room-space captured command origins and no-extra-tick room rebases;
raw controller callers still supply their actual command camera. Relic changes
freeze gameplay and never grant resources. Ordinary starts grant no Wolf relics;
saves stay v9. Selected [cross-form conversions](docs/player-wolf-cross-forms.md) now
retain actual Wolf snapshots and Bat packet attributes through real handshakes.
Water, other shake producers, all-ability contention
and full parity remain incomplete. Do not call Wolf or Alucard complete.

The [owned Wolf scene presentation](docs/wolf-native-presentation.md) now draws
real bank15 player/body packets, retained images, puffs/tiles and ordered common
capture. Preserve primitive-before-entity equal ties, previous-image sampling,
63x63 bounds, return player packets after body retirement, repeated-draw input
restoration and simulation-side capture flushing. Child culling follows actual
initialization/RNG and retires real rows. Selected playable Wolf now binds the
native contexts/body feedback and controls. Selected cross-form conversion now
runs; selected Wolf shakes now run. Water, other shake producers, global GPU
contention and full parity remain incomplete; Alucard is not complete.

The [selected Wolf secondary effects](docs/wolf-secondary-effects.md) now consume
actual controller factories/sounds and deferred head bubbles through owned children
and the shared stage pool, including real particle sentinel rows. Preserve captured
command-camera origins, body-camera bubble origins, next-tick factories, allocation
failure before RNG, original bubble fallthrough and no-tick rebases. US smoke's
unwritten status0x20000 birth branch fails explicitly. Native drawing now runs;
water providers remain unwired. Selected Wolf shakes now run; Alucard is incomplete.

Selected [Wolf combat ownership](docs/player-wolf-combat.md) now connects actual
front/head spells, INT/RNG/Mojo/critical metadata, retained group-three enemy
cooldown and native damage/death/EXP. Preserve source least-used counter increment
before forced enemyId3, end-of-tick actual identity recount, current pose geometry
and once-only front contact MP feedback. Late failures preserve body/MP/RNG.
Selected native scene/controls and child effects now run; Wolf and Alucard remain
incomplete. Private protected component sessions are not full encounters.

The [shared Wolf runtime](docs/wolf-shared-runtime.md) now interleaves actual
body17..30 with factory-selected Blink slots and commits Bat/Echo/Wolf rows in one
stage-pool transaction. Preserve current body bounds for later flashes, forced
replacement before birth, once-only MP/body feedback, retained environment rows
and no-extra-tick rebases. Late failures preserve runtime and character snapshots.
Selected native scene/render/combat/factories now consume these interfaces;
full sharing and Alucard remain incomplete.

The [owned Wolf player core](docs/player-wolf-core.md) connects selected original
entry/controller/8-pass collision/return to real generation-owned factories/Blink
and body interfaces. Preserve actual signals, pending entry, signed QCF timers and
captured facing, immutable mutable-cadence substitution, current-clock/once-per-tick
body feedback, original return without Bat/Mist's Y shift, and the explicit unchecked
missing-return-factory wait. Selected native scene/controls/relic switches now run;
selected cross-form conversion now runs. Water integration and Alucard remain incomplete.
BlinkWhite is not weapon/Mist slot16: use its actual factory-selected slot (selected
private fixture63). Interleave body17..30 and other children by actual slot; never
run every flash before the body.

The [linked Wolf body owner](docs/wolf-body-owner.md) now creates actual slots17..30
with generation/lineage checks, original body/head-animation ordering, real shared
root/environment row plans, MP feedback and no-extra-tick rebases. Preserve root
allocation retries, pending actual owners, common environment retention and stage
pool transaction checks. Feed its actual slot mask to other allocators; perform
original forced replacement explicitly before Begin. Spell/bubble intents confer
no attack/factory readiness by themselves. Selected native transformation/resource/
control/combat/render owners now consume them; Alucard remains incomplete.

The [Wolf transition component](docs/wolf-transition.md) reconstructs selected
entry0/8 and return0/1/2 to the real normal/stuck helper boundaries. Preserve actual
factory/Mist/signal gates, signed timer14 maxima, signal word wrap, all three
movement words, per-tick floor restoration and delayed ceiling readiness. Requests
confer no body allocation, successful return or lockout; the session must invoke
real helpers and commit ownership. Selected playable Wolf now does so; Alucard remains incomplete.

The [Wolf multipart/capture services](docs/wolf-presentation.md) now reconstruct
bank15 pieces with original SDK pivots, cropping, raw-color semantics, capacity
ordering and capture copies into the owned 63x63 target at512,448. Preserve
ascending priorities/reverse emission ties, original clear bit, outer texels and
transactional target ownership. Retained images share current pixels; never freeze
separate captures or mutate private source artwork. These components enable no
Wolf controls or actual transformation/body/combat ownership; Alucard remains incomplete.

The [Wolf BlinkWhite component](docs/wolf-blink.md) now reconstructs selected
entry23/return24/finish25, saved ordinary/Bat artwork before actual signal2, raw
Wolf capture bounds/US V wrap, exact limits, return handshake and shared Bat/Wolf
lock precedence. Preserve signed/inverted stripe bounds and unmirrored Wolf UVs.
Entry25 retains its palette; return1B switches white. Pending allocations never
signal readiness. Selected native Wolf now owns these body/transformation/capture/
combat/render interfaces; global sharing and Alucard remain incomplete.

The [Wolf retained-image component](docs/wolf-root-effect.md) now reconstructs
five retained-position rows and the live sixth overlay. Preserve first-selection
initialization, captured facing/positions, asymmetric fade and later hide, live
priority, nonmirrored UVs, byte-color wrap and shared current capture texture.
It owns no rows/pixels and signals no readiness itself. Selected native Wolf now
owns those lifetimes and controls; full sharing/Alucard/game/console parity remain incomplete.

The [Wolf root/head components](docs/wolf-root-head.md) now reconstruct original
root floor probing, slopes/ledge slides, frame/drawing choices and head attachment,
bite/charge/bark/bubble intent with separate private animation advancement. Retain
actual six-row allocation retries, screen-relative edge rules, ordered entity/
collision updates, full-width private offsets and signed image-cycle wrap. Negative
image indices select no retained image. No request establishes actual readiness,
attack/factory success or native form ownership. Selected native Wolf now consumes
these interfaces; full sharing/Alucard/game/console parity remain incomplete.

The [Wolf linked body component](docs/wolf-articulation.md) now reconstructs
selected slots18/19/21..30, private pose/frame choices, fractional attachments,
spine waves and final capture bounds. Preserve actual slot order around head20,
each child's first-update wave reset, two angular comparisons, retained frames
and rush-hit spell-thirteen MP/skid feedback. Attack intent creates no hitbox.
Selected native root/head/transformation/combat/render owners now consume these
interfaces; this does not establish full-form/Alucard/console parity.

The [Wolf active controller component](docs/wolf-controller.md) now reconstructs
selected idle/run/turn/crouch/jump/fall/recoil/slide/swim substates after the form
resource gate. Preserve US left-speed and wall quirks, separate landing tests,
ordered request origins, wrapping windows and exact-cost spell-four charge.
Return requests never manufacture body/readiness or transformation completion.
Selected native entry/return/body/combat/render owners and controls now run.
Cross-form/water/shake, Alucard and full parity remain incomplete.

The [Wolf movement/capture components](docs/wolf-motion.md) reconstruct articulated
raw bounds, sprint/slope/Up/swim jump ordering, signed-word angle wrap and selected
run-frame cadence. Preserve actual parts 17..30, US asymmetric one-sided clamps,
inverted supplied offsets and wrap-before-post-clamp ordering. The eventual owner
must provide the actual body generation; these components allocate no bodies and
enable no Wolf controls/relic switches. Preserve the original left-running cap
quirk in further controller work. Complete Wolf/Alucard remains unfinished.

Selected [Bat primitive sharing](docs/bat-primitive-sharing.md) now reserves real
stage-pool rows for BlinkWhite, trails and Echo. Preserve eight-row US Blink
retries without fake readiness, one-row trail retirement, real Echo exhaustion,
child-slot release ordering, tagged generation ownership and atomic whole-Bat
commits. Room entry retains leases; full stage/Continue resets them. Mist/legacy
sharing, original packed-stage capacity differences, all-ability factories/groups,
Wolf and full route/console parity remain incomplete.

[Selected Echo of Bat](docs/player-echo-of-bat.md) now connects owned A/Xbox Y
commands, actual factory/slot47 and forty-nine rows shared with the stage primitive
pool, original wave packets, quarter-add rendering and Force spell-ten native combat.
Preserve captured facing/priority, live rotation, SDK rounding, birth-only Force,
strict attack cutoff, group7..10 cooldown across Fire/Echo generations, actual
exhaustion and no-extra-tick room rebases. Closing menus must not cast Echo.
Ordinary starts grant no relics; saves remain v9. Mist/legacy primitive sharing,
all-ability factory/groups, Wolf and route/console parity remain incomplete. The
[Echo component](docs/bat-echo.md) retains its scoped comparison evidence.
Do not call Bat, Alucard, the castle or the game complete.

[Selected Fire of Bat](docs/player-fire-of-bat.md) now connects owned hand-attack
casts, real factories/slots32..47, private spell-nine attacks/groups7..10, impact
feedback, bank-nine animation/artwork, SDK scale, flash/sound and ordinary use
timer ten. Preserve generation checks, actual Gas/Fire availability, cooldown across
births, launch fractions, original recovery and paid casts canceled by later MP
exhaustion. Timer ten is not invincibility. Ordinary starts grant no relics;
Wolf and full shared-factory/primitive/route/console parity remain incomplete.
The [fireball motion component](docs/bat-fireball-motion.md) retains its scoped
comparison evidence. Do not call Bat, Alucard, the castle or the game complete.

[Selected Wing Smash](docs/player-wing-smash.md) now connects owned Bat commands,
private spell/critical-hit data, real factories/children, fan/trails, native enemy
combat/EXP and original return. Preserve actual slot masks shared with Gas Cloud,
critical retries, per-group cooldown across generations, full-parameter flash
deduplication, original retained phase four and rebases without extra ticks.
Pending factories never signal readiness. Ordinary starts grant no relics; saves
remain v9. All-ability factory/group/primitive contention, afterimages, dust/shake,
Wolf and whole-route/console parity remain incomplete. Do not call Bat,
Alucard, the castle or the game complete.

The [player-ability factory component](docs/player-ability-factories.md) now
reconstructs private blueprint scheduling and actual-mask slot selection. It owns
no second pool and enables no abilities by itself. Preserve critical retries,
optional partial retirement, first/later origin ordering, per-cycle versus lifetime
parameters, forced weapon replacement and valid zero-identity slot clears. Actual
factory reservation, child generations, behaviors and shared attack/primitive
ownership remain the native session's responsibility. The [selected Wing Smash runtime](docs/player-wing-smash.md)
now owns real Bat factories/children; full all-ability sharing remains incomplete.

[Bat special presentation components](docs/bat-special-presentation.md) now run
selected Fire/Wing flashes and shrinking trails. Their [selected Wing Smash integration](docs/player-wing-smash.md) now runs live;
Selected Fire now runs through its actual owner. Preserve fan/strip mirror differences, original 0x7009 fallthrough and
retained phase four, real transformation-lock removal, live frame/frozen trail
orientation and scale-before-SDK-rotation. Full shared factories/combat remain incomplete.

The [Bat command/motion component](docs/bat-command-motion.md) now reconstructs
Wing Smash input and Wing/Fire controller paths after the form resource gate.
It emits original requests; [selected Wing Smash](docs/player-wing-smash.md) now consumes
those requests in gameplay. Selected Echo now runs through its actual owner.
Preserve exact directional words, wrapping timers, US repeat casts, exact-cost
spell MP and slope ordering. Actual child/combat/visual ownership must be wired
before enabling further ability controls.

Selected [playable Bat](docs/player-bat.md) now connects owned E/Xbox RB entry,
ordinary flight, real Blink/sensor restoration, MP, collision and damage. Preserve
actual generation ownership, per-frame rotation flags, shared settling, original
16/24-tick lockouts and real Mist cross-conversion. Bat uses step five; its return
uses step nine. Raw textured body packets must ignore vertex colors. Room changes
rebase without advancing effects; stage initialization/Continue restore normal form.
Ordinary starts grant no relics. Selected Wing Smash now runs; Wolf, shared effect contention,
full effects/audio and whole-route/console parity remain incomplete.

The [Bat presentation foundation](docs/player-bat-presentation.md) now decodes
private bank-thirteen/separate-ARC artwork and runs actual selected BlinkWhite
entry/return/finishing effects used by selected playable Bat. Preserve real
signal two/three, sensor handshake, shared phase/lock ordering, status-held finishing
flash, signed-word geometry and twelve-bit SDK rounding. Upload actual pixels
separately from Mist capture profiles; retain original margins and immutable input
artwork. The owner must apply Bat's live settling pivot. Start/Select use original
US words 0x0800/0x0100 internally, leaving shoulder bits available; keep SDL keyboard/
gamepad menu controls unchanged. Runtime inputs/traces stay private. Shared effects,
full collision/damage/console parity remain incomplete.

Selected [Gas Cloud combat](docs/player-gas-cloud.md) now connects owned entry,
private spell/INT/RNG/Mojo Mail initialization, actual cloud hitboxes and shared
within-cloud enemy cooldown to native damage/death/EXP. Preserve the original
seventeen-child request's sixteen-slot allocation limit, negative midpoint
truncation and one RNG value per actual child. Relic toggles suspend/resume existing
children; enabling midway through Mist never creates a missing factory. Room
rebases refresh positions without advancing the visual; normal/stage reset retires
transient children. Actual Gas/Bat child masks now share selected slot availability; its group-four
channel remains separate from other ability allocators. Shared factory/group contention,
ordinary acquisition and whole-route/console parity remain incomplete.

Selected [Mist traversal](docs/player-mist.md) now connects owned relic/MP gates,
steering, compact sensors, barrier escape, real sprite capture/cloud/palette jobs,
restoration and cramped-space recovery. Q/Xbox LB toggles owned Mist; ordinary
starts grant no new relics. Use the absolute simulation clock, freeze original MP
recovery in transformed states, and retain generation ownership. Never substitute
a timer for the actual visual phase-5 signal; normal return releases the real child.
Mist's four birth rand calls stage the existing character libc cursor and commit
after the effect succeeds; never advance the separate stage Random cursor.
Later updates/menu/draw/no-tick rebases consume no birth calls. Full cross-owner
libc/Gas Cloud birth ordering remains unfinished; no new RNG/save fields.
Keep the original 16/24-tick entry/return lockouts, slope bit 0x8000 and flying
bottom room window. Wolf, shared effect contention,
restoration flashes, full raster/audio/parity and ordinary acquisition remain
incomplete. Do not call Alucard, the castle, its areas or the game complete.

[Mist motion](docs/mist-motion.md), [form collision](docs/form-restoration-collision.md),
[visual lifecycle](docs/mist-effect.md) and [capture/palettes](docs/mist-presentation.md)
retain their scoped component comparisons. Preserve raw tile distances/asymmetric
masks, successful zero-shift ejection and the original second-probe Mist quirk.
Camera/room rebases move retained ring fractions even when no child is alive;
live anchors/quads move separately. Cached capture geometry remains unchanged.
Initialize owned player palettes through original character-start job 22: raw
F_GAME palettes contain placeholder Mist colors. Refresh current cape colors
without resetting live fade jobs. Original full stage initialization retires form
entities and restores normal Alucard; retained global ring/color state survives.

The [transformation resource service](docs/transformation-resources.md) retains
strict MP thresholds and absolute wrapping-timer charge cadence. Check-only never
charges and found-but-disabled relics block entry even on an unbilled tick. Apply
the reported cost once to each owning snapshot. Selected Bat/Mist now run; Wolf remains unavailable. Resource component checks
do not establish full-form parity.

[Connected Olrox’s Quarters progression](docs/olrox-connections.md) now links
Marble Gallery and Chapel, the original eastern Olrox boss override, return exits
and defeated-arena revisits. Gate the encounter on its defeat time, not visited or
cutscene bits. Preserve original high exits and private route records; grant no
abilities to ordinary starts. Isolated ordinary-character connector checks and
protected strength/Gravity Boots boss fixtures are distinct evidence. Save-room
transfers are explicit; do not claim continuous traversal, ordinary acquisition,
whole-encounter or console parity. Colosseum/teleport destinations remain gated.

The selected [Olrox furnishings](docs/olrox-furnishings.md) now add placed table,
seats/candelabra, original adjacent-group destruction, thirteen debris variants
and normal sitting controls. Retain private profiles, pure-Up triggering, fractional
motion, floor sensors and rotation through explosion conversion. Validate adjacent
groups before mutation; exhausted visual pools never replace unrelated actors.
Explicit-entry/protected native checks are not full-route or console parity.

The selected [Olrox encounter](docs/olrox-encounter.md) now connects both forms,
real morph/body/head/arm/attack updates, shared damage, independent hand recovery,
captured-fragment defeat, Life Max Up conversion and arena release. Preserve the
seven-part head-HP ring (roles 0..6), infinite-HP cooldown rules, independent arms/
hands and packet-16 final body capture. Only final victory records the boss time;
first-form transition never awards victory. Saved defeat times omit boss/doors
on a fresh arena. Full effects/audio/shake/flash,
Alucard afterimages and uninterrupted encounter/console parity remain incomplete.
Do not call the boss presentation, area, castle, Alucard or game complete.

[Body/movement](docs/olrox-monster-body.md), [joint motion](docs/olrox-monster-motion.md),
[geometry](docs/olrox-monster-geometry.md), [appendages](docs/olrox-monster-appendages.md)
and [head/attacks](docs/olrox-monster-attacks.md) retain original strict waits,
first-takeoff leg-zero ordering, signed word/fixed-point wrap, the four-pixel
anchored/airborne origin difference, fractional attachments and packed roar timing.
Keep stage sine samples, SDK quarter-wave trig and matrix-pair math distinct.
Preserve signed-controller/unsigned-geometry extension bytes, released projectile
lifetimes, triangle geometry, particle metadata and original RNG/pool order.

The [morph](docs/olrox-morph-controller.md) and [owned targets](docs/olrox-render-targets.md)
now run in gameplay. Flush pending offscreen writes before the next simulation
step can overwrite one-tick capture commands. Preserve 127x256 then 128x128 bounds,
VRAM row511, immutable source images, borrowed surface lifetimes and ordered
replacement/restoration. Modes3/4 permit original replacement/repeated restores;
strict modes1/2 retain nesting checks. Fresh BO0 owners reproduce the bounded
original menu-cleared actor atlas; other missing texture references still fail.

Actor updates use working copies: compare slot, identity, generation and lineage,
not C addresses. Keep bounded reserved handoffs and do not replace unrelated actors.
Preserve the documented initialized midpoint metadata and omitted invalid optional
burst concessions in [encounter scope](docs/olrox-encounter.md); do not reintroduce
unwritten scratch bytes or exhausted/null packet access. Never fake readiness.
Original XA92 fade requests remain separate from sound effects; later track317
restores stream gain. Room entry resets articulation and morph lifetime caches.

[Humanoid](docs/olrox-first-form.md), [orbs/portals](docs/olrox-orbs-portals.md),
[fire/effects](docs/olrox-fire-effects.md), [skull columns](docs/olrox-skull-columns.md)
and [arena doors](docs/olrox-arena.md) retain their own original BO0 tables/identities.
Component instruction comparisons do not establish a complete encounter or console
replay. Focused component integration uses an explicit first-form death fixture;
a separate native/actual-app sequence defeats both forms by ordinary development
Gladius damage and collects the original +5 HP reward. It uses explicit arena entry
and contact protection, not uninterrupted traversal or an unmodified encounter.
All automated runtime probes remain muted and preserve ordinary user saves.

Selected [NO2 courtyard scenery](docs/olrox-courtyard.md) now renders private
house models, repeated backgrounds, foreground/backdrop, linked lamps and glow.
Preserve matrix-pair projection (distinct from quarter-wave trigonometry), face
winding/signed midpoint division, effect-slot order and copied palette commands.
Palette red increments preserve color zero, other channels and mask bits; never
mutate borrowed assets. The framebuffer effect remains incomplete; selected Olrox
and connected travel now run within their documented limits. Explicit start/protection
checks are not whole-route/console parity.

Selected [NO2 stone doors](docs/olrox-stone-doors.md) now retain the original
forced entry/open/close sequence, alternate artwork and collision columns.
They are ordinary doors, distinct from the magical Jewel barrier. Preserve the
quarter-wave SDK trigonometry, right-side shading, retained edge UV at `0xA00`,
original sound requests and checked primitive/tile ownership. Selected NO2 travel
now runs; explicit placed-door checks are not whole-route/console parity.

The selected [relic pause controls](docs/relic-menu.md) now switch owned Cube of
Zoe, Soul of Bat, Gravity Boots, Leap Stone, Jewel of Open, Mist, Power of Mist, Gas Cloud and
the Sword Card (other cards off; menu close loads the familiar). Preserve found bits, US cursor
mapping (skip identities 23/24), paused simulation and immediate player snapshots.
Other abilities and familiar cards remain unavailable; never grant ownership or reset
transient movement/MP when switching. Version-nine saves retain disabled relics.
Private grants/transfers are not ordinary acquisition or whole-route parity.

The selected [Gravity Boots service](docs/player-gravity-boots.md) now runs the
original command, strict MP threshold/cost, high-jump/ceiling states and owned
beam. Preserve sixteen-bit command timers, signed duration comparisons, the late
ceiling facing flip, and eight collision passes for high jumps/fast horizontal
motion. Successful player steps report one MP cost to ordinary character progress.
Selected [ceiling dust/shake](docs/player-gravity-impact.md) now run. Dithering,
full shared factory contention and complete ability parity remain incomplete. Native ceiling checks use explicit relic/protection grants; no default
grant or ordinary acquisition claim is authorized by that evidence.

The selected [Leap Stone double jump](docs/player-leap-stone.md) now uses a copied,
validated relic snapshot from session progress, original Jump/Fall semantics and
one use per airborne interval. Never grant it by default or refill it during a
room change/snapshot update. Full dive-kick/Gravity Boots parity and other abilities remain
incomplete; positioned/private-grant checks are not ordinary acquisition parity.
The selected [dive kick](docs/player-dive-kick.md) adds an owned empty-hand hitbox,
generation-checked feedback and original delayed rebound/landing. Preserve enemy
cooldown across child generations and legacy Chapel-ball groups. Original shared
factory capacity/dynamic attack groups and complete effects remain incomplete.

The selected [normal-form boss grab service](docs/player-boss-grab.md) now provides
Hippogryph's start/held/damage/clear-to-drop handshake, original grabbed animation
and pain requests, collision/input hold, defense/poison damage and selected lethal
completion. Thrown-release/stone grabs and full grab effects remain incomplete.
The [BO5 attack adapters](docs/hippogryph-attacks.md) add owned pose hitboxes,
flying objects, flame/particles and bounded effect pools, with private instruction-tick
comparisons. The [Hippogryph encounter](docs/hippogryph-body.md) reconstructs normal motion/attack/grab
states, linked hit handling, original defeat effects, arena doors/music and Life Max Up reward
conversion. Maria's original dialogue/events now connect the reward-watching manager, actor,
speech, private flashback pictures and player/camera control. Shared original clouds/sky load
BO5's separate private tables and render behind its tile layers. Version 9 saves retain the
original cutscene completion word, with explicit v1–v8 migrations.
[Chapel connections](docs/chapel-connections.md) now link Laboratory, Chapel saves and BO5
from both sides, with defeated-arena revisits. Protected strength-grant boss fixtures and an
ordinary-character entry/save-room route do not establish whole-route or console parity.
Further Chapel exits and full encounter/presentation parity remain incomplete.

The internal [Olrox's Quarters foundation](docs/olrox-quarter-runtime.md) adds its
own NO2 resources/music, common pickups/doors/camera, breakables/debris, decorations,
Skelerang/boomerangs, selected Bloody Zombies and blood-effect adapters. Keep NO2
identities distinct: zombie 0x2D, blood 0x2E/0x2F, Owl Knight sword/target 0x2B/0x2C.
Selected stage-4/BO0 travel now runs within the documented limits. Public synthetic
and controlled private checks are not whole-area or
console parity. Its ordinary save room uses the existing version-nine format.
Selected [NO2 prisoners and spikes](docs/olrox-prisoners-spikes.md) now retain
owned shadows, original appearance/rattle/retreat phases, temporary environmental
contact and equipped tile destruction. Preserve NO2's mixed actual-width/96-tile
indexing and its separate identities/tables; do not reuse Chapel's adjacent damage
ownership or orientation rules. Unknown broken replacements fail before mutation.
Selected [NO2 Valhalla Knights](docs/olrox-valhalla.md) now run charge/leap/recovery,
linked body damage, independent weapon contact and thirteen-piece deaths. Preserve
the two different child roles, generation ownership, private frame hitboxes and
simulation-frame effect phase. Component/protected checks are not full encounter parity.

Selected [NO2 Hammer/Blade encounters](docs/olrox-giant-knights.md) now run their
private articulated poses, independent weapons, linked physical damage and staged
deaths. Preserve copied-root anchoring, signed SDK overextension and generation
ownership after Blade releases its sword. Optional trail exhaustion omits only
visuals; the original small-Y impact shake remains incomplete. Ordinary NO2 layouts
do not place the compiled Owl/Owl Knight or Gurkha families. Selected stage-4 travel
now runs; component/protected checks do not establish route/console parity.

Selected [NO2 Spectral Sword combat](docs/olrox-spectral-sword.md) now runs its
fifteen distinct weapons, three formations, staggered throws, independent weapon
HP, count-once deaths, body defeat and owned aura. Preserve NO2 identities 3E/3F/40,
camera-relative thresholds, surviving-weapon projection cursor and private tables;
Chapel's encounter differs. Component/protected checks are not route/console parity.
Selected stage-4 travel and Olrox now run within their documented limits.

Selected [NO2 secrets](docs/olrox-secrets.md) now connect the three-hit wall,
128-HP ceiling, original tile patches and selected paired debris/dust. Flag32's
wall discovery/aftermath bits and flag33's ceiling boolean persist in v9 saves
and feed the existing map renderer. Retain infinite HP0x7FFF, paired primitive
ownership, fractions and distinct allocation fallbacks. The native wall crossing
uses ordinary Gladius attacks. The initial ceiling fixture used explicit positioning;
the selected Gravity Boots check now reaches it with normal commands and relic/
protection grants. Neither proves whole-area/ability/console parity. Selected stage-4
travel and Olrox now run within their documented limits.

## Purpose and priorities

Build a native, personal and educational *Symphony of the Night* port that
preserves the original experience and creates a foundation for later improvements.
The C17 SDL3 app includes a limited Laboratory actor checkpoint, not a complete
working game port. `--laboratory` launches our executable with runtime data;
`--laboratory-room 3` selects the zombie corridor. Selected actors animate and
Bloody Zombies run active motion/attack animations. Selected empty-hand, Basilard/Gladius/Mace and ordinary Alucard sword/shield combat, physical damage, candle drops, and collection are
integrated; read [combat scope](docs/laboratory-combat.md). Other equipped weapons,
complete effects/audio, castle scripts, full map parity, and full progression remain incomplete.
Its original `modules/data/` library validates private core files; read
[docs/private-data.md](docs/private-data.md) before extending that boundary.
Its `modules/render/` library presents RGBA images through embedded OpenGL/ES
shaders; [docs/opengl-presentation.md](docs/opengl-presentation.md) describes
the asset preview and graphics checks. It does not implement PS1 game logic.
The `modules/core/` library now contains adapted reconstructed collision and
graphics-decoding algorithms. Read [docs/game-core.md](docs/game-core.md) for
their provenance and scope. Normal Alucard movement and collision sensors are
integrated; [movement integration](docs/laboratory-movement.md) describes their limits.
The core stage parser and on-demand data resources are documented in
[docs/stage-data.md](docs/stage-data.md); runtime tables must stay in private files.
Actor placement decoding and scrolling/persistence state are described in
[docs/laboratory-actors.md](docs/laboratory-actors.md), including selected behavior
and rendering integration. Do not describe visual attack animations as implemented combat.
Runtime tile/sprite presentation and the animation cursor are described in
[docs/laboratory-presentation.md](docs/laboratory-presentation.md). Its tile
preview is a graphics diagnostic, not a gameplay playtest.
Read [docs/project-goals.md](docs/project-goals.md) before planning conversion work.
The selected route is a **decompilation-based native port**; read the current
[feasibility report](docs/native-port-plan.md) and pinned
[reference manifest](config/reference-us.json) before importing code or data.

- **Phase 1: authenticity first.** Preserve gameplay, timing, controls, audiovisual
  behavior, progression, and save behavior. Prove the desktop experience first.
  Keep web and mobile authentic too; make only required platform concessions,
  such as touch controls where a physical controller or keyboard is unavailable.
  Document each concession and test that it preserves game behavior.
- Defer widescreen, uncapped rendering, scanline shaders, save/load states,
  cheats, and other enhancements until the authentic foundation is proven.
  Do not silently fix original behavior or mix enhancements into parity work.
- **C throughout.** Generated or mechanically translated C may be a temporary
  bridge during conversion. Isolate it, document its provenance and limitations,
  and test its behavior. The planned **2.0** milestone replaces such transitional
  code with human-readable, idiomatic C; it is not a future switch from C++ to C.
- This task history does not authorize every future conversion step. Scope work
  to the current request and report the actual implemented state accurately.

## Language, ownership, and safety

- Write first-party native code in **C17**, not C++. Use `src/main.c` as the
  single entrypoint. Platform-required Java/Objective-C glue and build/test
  scripts are acceptable; keep game logic in C. A dependency's platform backend
  may require C++ internally (SDL's Android HID backend does); this does not
  authorize first-party C++. Keep any required runtime statically linked.
- Favor simple, explicit code that a human can read and change safely. Apply
  SOLID through focused modules, small interfaces, explicit ownership, and
  injected dependencies. Use structs and function pointers where variation
  requires them; avoid speculative abstractions and object-system emulation.
- Initialize variables at declaration to useful defaults. Use `{0}` or checked
  zeroed allocation for aggregates; use explicit invalid sentinels where zero is
  a valid resource. Never rely on uninitialized storage or implicit ownership.
- Prefer arena allocation for groups of objects with a shared lifetime, such
  as a level or frame. Check capacity and alignment, define reset boundaries,
  and never retain pointers beyond an arena's lifetime. A small, separately
  owned allocation is appropriate when an arena would add needless machinery.
- Prefer SDL3's standard-library, allocation, logging, and I/O functions for
  portable behavior. Match each allocation with its allocator's release API.
- Use explicit types: fixed-width integers or SDL integer types for defined
  widths, `size_t` for memory sizes, and SDL's specified types at API boundaries.
  Do not assume the width of `int`, `long`, pointers, or enum storage. Validate
  narrowing conversions, byte order, alignment, and signed/unsigned arithmetic.
- Follow secure C practices. Validate external input at boundaries; check sizes,
  overflow, offsets, indices, allocation results, and I/O results before use.
  Avoid unchecked copies, unbounded formatting, format-string injection,
  use-after-free, and accidental aliasing or lifetime dependencies.
- Fail fast with clear, actionable error messages and explicit failure results.
  Include the operation and relevant context; clean up partial initialization.
  Do not continue with corrupt state or hide errors behind a successful result.
- Keep warnings as errors for project code. Do not add warning suppressions to
  conceal defects. Use the checked-in Microsoft clang-format layout.

## Platform behavior

- Link SDL3 statically from an integrity-checked, pinned source release. The
  Android JNI shared library contains static SDL3; OS libraries remain external.
- Use **OpenGL** for the game renderer, with OpenGL ES/WebGL adaptations where
  the platform requires them. The bootstrap's SDL debug-text display is not the
  final renderer. Do not replace the rendering direction with another backend
  without a specific requirement to do so.
- Embed shader source as C `.h` headers or byte arrays and compile/link it at
  runtime. Ship no filesystem-dependent shaders. Validate capabilities and
  report shader compile/link errors clearly; keep desktop and ES variants explicit.
- Keep original simulation timing independent from presentation. Browser frame
  pacing belongs to SDL's callback loop; do not block the browser with a game loop.
- Put saves, settings, and persistent state under a location obtained through
  **`SDL_GetPrefPath`**. Never assume the working directory or install directory
  is writable. Check directory and file operations, protect existing saves
  against failed writes, and test format migrations. Web persistence needs an
  explicit persistent-storage adapter and reload tests.
- Use SDL's gamepad abstraction with a ubiquitous **Xbox-layout controller** as
  the default reference. Preserve the original game actions, document mappings,
  and support rebinding. Prefer A/B/X/Y, shoulders, triggers, d-pad, and sticks
  over raw device indices. Use vibration when supported; lack of vibration must
  never prevent play. Keep desktop keyboard and required mobile touch fallbacks.

## Tests, CI, and builds

- Automated tests are essential. Add regression coverage for behavior changes,
  failure paths, parsing, ownership, and platform adaptations. Fix failures;
  do not weaken tests to obtain a green build.
- Use reproducible input/replay comparisons to establish authenticity. Preserve
  simulation results independently of render rate. Use synthetic or original
  test fixtures in public CI. Asset-dependent parity tests use the user's local
  dump; report missing-input tests as skipped, never as proof that parity passed.
- Keep Linux, macOS, Windows, browser, Android, and iOS build/runtime checks
  working. Run **AddressSanitizer in Linux CI**, with UndefinedBehaviorSanitizer
  alongside it. Treat sanitizer reports as bugs and retain failure diagnostics.
- CMake owns build logic; GNU Make wraps it. Use `make`, `make test`,
  `make sanitizer` (Linux), `make package`, and platform targets. List sources
  explicitly; reusable code belongs in focused modules as the project grows.
- `make` and `make dev` build Debug and launch the supported Castle Entrance
  beginning; `make release` builds and launches Release. Build-only automation
  uses `make debug-build`, `make release-build` or CMake. `make debug` launches
  through LLDB. Automated runtime checks still require muted audio and LLDB;
  the original title/prologue flow remains incomplete.
- Pin dependencies and avoid competing utility stacks where SDL3 suffices.
  Build and release packaging must not require private game inputs.
- Keep local validation artifacts temporary. After recording results, remove standalone
  probe binaries/debug symbols, large raw captures/traces, obsolete platform builds and
  unused downloaded toolchains. Preserve source/probe scripts, concise result logs,
  private game inputs and user saves. Retain only build trees needed for active work;
  do not accumulate a permanent build or binary for every probe or feature batch.

## Source and release policy

- Never commit or publish ROMs, BIOS images, extracted game binaries, original
  game source dumps, or copyrighted game assets. Users supply their own game
  dump. Keep private inputs outside the checkout or in ignored `local/`.
  Document source/tooling provenance and compatible licensing for public code.
- Run `python3 scripts/check_repository.py` and review the staged diff before
  committing or publishing. Do not weaken the guard to admit game data. Keep
  private data out of CI artifacts, logs, release archives, and test fixtures.
- Use **Semantic Versioning**. Edit root `VERSION` only. Stay on **0.x.x** until
  the project provides a competent, stable release; **1.x.x** represents that
  stable foundation. The intended **2.0** milestone is the idiomatic C rewrite.
  Version saves and state formats explicitly; do not infer their compatibility
  from the application version alone. Annotated release tags `vX.Y.Z` must match
  `VERSION`; a feature branch does not itself require a release or version bump.

## Documentation and collaboration

- Maintain an idiomatic, accurate README for setup, build, play, tests, and
  packaging. Put feature requirements, design decisions, parity criteria, and
  platform concessions under `docs/` and link them from relevant entrypoints.
- Use professional **Doxygen** comments for source interfaces: purpose,
  parameters, returns, ownership/lifetimes, preconditions, and failure behavior.
  Internal comments should explain durable invariants or tradeoffs, not narrate
  statements or serve as a change log.
- Work on **feature branches** and use **`gh` to open pull requests**. Keep each
  PR focused and reviewable; do not merge unless the user authorizes it.
- Current game-integration work uses stacked PRs. Leave them unmerged until the
  user playtests and verifies the project build; merge dependencies first only
  after that approval. This supersedes earlier blanket permission to merge on CI.
  Exception: for the overnight batch explicitly authorized on 2026-10-25, the user
  waived manual playtests and authorized merging passing PRs bottom-up. Continue
  automated/private validation; this exception is not an authenticity claim and
  does not waive the normal gate for later work after that batch.
  On 2026-10-26 the user continued this authorization: work autonomously until the
  castle and Alucard's abilities are complete. Continue focused PR batches and
  bottom-up merges after all exact-head CI checks pass, without manual-playtest
  pauses. Preserve automated/private validation and report incomplete scope honestly.
- **Mute automated playtests by default.** Set `SDL_AUDIO_DRIVER=dummy` when
  launching the app or private runtime probes, so unexpected sound does not
  disturb the user. Enable audible output only when specifically testing audio;
  state that purpose before starting. This is a playtest-launch preference, not
  a change to the game's normal audio defaults. Local `play-test.sh` defaults to
  muted output and accepts `--audio` for an intentional audible check.
- Give game build/run instructions through **LLDB**, so a crash leaves a useful
  stack trace (`thread backtrace all`). Playtests must launch this repository's
  executable built through its CMake/Make workflow.
- Be succinct and use active voice in responses and PR descriptions. Generally
  write PRs as **context > problem > solution**, emphasizing user experience.
  Include a brief validation result. Leave implementation details in the code
  unless a reviewer needs them to understand a material tradeoff.

The [physical death/game-over flow](docs/laboratory-game-over.md) now runs the selected
original presentation and returns to the native Continue shell after its fade. Original title/file selection,
elemental deaths and complete effects remain incomplete. The
[save-room checkpoint](docs/laboratory-save-rooms.md) now restores HP/MP and persists
supported progress to fifteen native slots; `--laboratory-continue SLOT` loads one.
The native slot browser supports Continue; original title/card menus, full-game progression
persistence, and PS1 cards remain incomplete.

The [original HUD](docs/laboratory-hud.md) presents HP, MP, hearts, and the collected
subweapon from private artwork/tables. Supported progression now updates maxima and MP recovery.

The [native sound service](docs/laboratory-audio.md) now plays selected original
combat sound effects and Laboratory music; reverb and full audio parity remain incomplete.

The [character inventory](docs/character-inventory.md) now connects original pickups,
selected pause/equipment panels, body stats, physical defense, and cape colors.
Selected relic controls now run; spells/settings/familiar menus remain unavailable.
Version 9 saves retain this supported progress; versions 1/2/3/4 migrate explicitly from their earlier profiles.

[Character progression](docs/character-progression.md) now awards level-adjusted
enemy EXP, original level/resource/stat gains, selected MP recovery, and permanent
Heart/Life Max Up collection. Complete level-up effects and familiar EXP remain incomplete.

The [selected subweapon service](docs/laboratory-subweapons.md) integrates dagger, axe,
holy water, and stopwatch; other collected subweapons remain explicitly unsupported.
Named `--laboratory-test-profile` fixtures are development grants, not original new-game starts.

Version 9 [ordinary native saves](docs/native-saves.md) now retain supported character
inventory/equipment, progression/maxima, permanent Laboratory pickup bits, and four
owned RNG streams, explored map cells, and permanent Laboratory events. Versions 1/2/3/4/5
migrate explicitly. Transient actor/projectile state, full castle scripts, and PS1
memory-card support remain incomplete.

The [Laboratory traversal milestone](docs/laboratory-world.md) now adds Skeleton,
Bone Scimitar, Blood Skeleton, Axe Knight, and Spittle Bone behavior/projectiles.
Room switches/spikes, boxes, lifts, selected doors, cannon/secret obstacles now run.
Normal camera follow and room locks now advance independently of rendering.
The paired boss encounter now includes attacks, coordination, defeat, reward, and music changes.
Ordinary saves retain map exploration and permanent Laboratory events. Full map parity, Maria/castle
scripts, whole-encounter replay parity, full effects/audio, and other stages remain incomplete;
do not call the area complete.

[Castle stage loading](docs/castle-stage-loading.md) now verifies 50 canonical stage
overlay/artwork pairs on demand. `--inspect-stage CODE` is a headless data diagnostic,
not native gameplay support or permission to send the player to an unintegrated stage.

The [exploration map](docs/castle-map.md) now opens with Tab/M or Xbox Back/Select.
It uses private blueprint/palette/passage data and saved exploration; full map and
paused-stage presentation parity remain incomplete. Missing F_MAP.BIN fails explicitly on first use.

The [native session shell](docs/native-session-flow.md) opens with `--play` and
returns after game over. It supports read-only slot selection and transactional
Continue, not the original title/new-game flow. Failed loads preserve the current
owner and display an error. Future stage destinations still need native actor/script integration.

The first [Entrance integration slice](docs/entrance-runtime.md) adds a stage-65
development entry (`--entrance`, `--entrance-room 0..27`), bats, floor zombies,
Wargs, both Mermen and their selected projectiles/splashes, and shared actors.
[Death's selected event](docs/entrance-death.md) now runs with original speech,
private portraits/text, targeted theft, and return of player control. Unused Fire
Wargs and additional later-Entrance enemies remain unintegrated.
[Connected stages](docs/connected-stages.md) now travel Entrance → Laboratory →
later Entrance with character/world/time/RNG retention and stage-aware saves. Keep stage identities
and original actor IDs distinct; do not describe the Entrance as complete.

[Original opening equipment](docs/opening-equipment.md) now equips the first Entrance
checkpoint with Alucard's six original items and integrates ordinary sword/shield
behavior, Dragon Helm defense reduction, Twilight glow, and actual equipped theft.
Prologue-derived stats, starting relics, command attacks, shield-rod spells and full
presentation remain incomplete; do not call this original new-game initialization.

The [relic progression service](docs/relic-progression.md) collects original stage orbs,
retains found/active ownership in v6 saves, and gates candle drops on Cube of Zoe.
Selected permanent pickups retain original castle bits across stage changes.
Selected Leap Stone/Gravity Boots and relic switches now run; other movement
abilities and complete acquisition effects remain incomplete.

The [Laboratory Maria encounter](docs/laboratory-maria.md) now reads private scripts,
animates Maria and her door, streams five speech clips, pauses/resumes music, and
returns control. The met-Maria flag persists in ordinary saves. CD timing parity,
complete dialogue effects, and full castle scripts remain incomplete.

The [early castle profiles](docs/early-castle-stages.md) add Marble Gallery/Outer Wall
(stage 0/1), shared Skeletons, Marble green Axe Knights, stage music, ordinary saves
and connectors. New per-area enemies, mechanisms, boss transfers and whole-route
parity remain incomplete; these are development profiles, not completed areas.

Selected [Marble encounters](docs/marble-gallery-enemies.md) and
[linked enemies](docs/marble-linked-enemies.md) now include Flea Men, Slingers, Ouija
Tables, Stone Roses, Skelerang and Diplocephalus with selected projectiles and death
behavior. Public synthetic and private explicit-room LLDB checks cover these paths;
continuous-route/console parity and complete area actors/effects remain incomplete.

The [early Marble clock room](docs/marble-clock-room.md) now runs timed statues,
stopwatch gating, hands, birdcages, selected floor/shadow/pendulum behavior, and clock
audio with original volume/pan. The later ring ritual and full effects/priority/timing
parity remain incomplete. Stage services consume elapsed ticks, not whole seconds.

Selected [Outer Wall ranged encounters](docs/outer-wall-ranged-enemies.md) now run
coordinated Bone Musket groups and Bone Archers, shots/arrows, and selected death
paths. Other Wall enemies/mechanisms, status damage and whole-route parity remain
incomplete. Private native checks use explicit protected room fixtures.

The [Outer Wall Spear Guard](docs/outer-wall-spear-guard.md) now adds selected melee,
blocking, thrown-spear and death rules. The copied player state exposes the original
selected four-tick subweapon startup interval for enemy decisions. Other unsupported
actions/status effects remain absent; private native checks use a protected fixture.

The [Outer Wall Sword Lord](docs/outer-wall-sword-lord.md) now runs selected attacks,
its separate weapon hitbox, bounded dust, death pulse and falling/fading fragments.
Private explicit-room checks use a protected player; whole-route and console parity
remain incomplete. Original tables and assets stay private.

The [Outer Wall Blue Axe Knight](docs/outer-wall-blue-knight.md) now runs selected
armored/damaged attacks, its separate weapon actor, axes and both death variants.
Green/blue fragment deaths retain paired primitive reservations. Private protected
fixtures are not continuous-route or console parity evidence.

The [Outer Wall elevator](docs/outer-wall-elevator.md) now runs selected three-stop
rides, remote calls, door tile/geometry changes, strike activation, gears and pressure
platform, with two persistent v6 mechanism flags. Sound supports channel 22/23 gain
updates without retriggering loops. Complete elevator effects/priority and whole-route
parity remain incomplete. Private protected fixtures are not console parity evidence.

The [Entrance water-room rocks](docs/entrance-rocks.md) now integrate two-hit breaks,
selected debris, the food drop, foreground/background tile changes and persistent
NO3/NP3 broken-side flags. Background tiles have their own owned room copy. The
later wolf/bat ritual remains unavailable; never grant its prerequisite bits implicitly.

The [Entrance stair section](docs/entrance-stairway.md) now runs its sixteen-HP break,
food drop, selected tile/debris effect and permanent restoration in NO3/NP3. Tile
art metadata is borrowed per room; room entry clears it. Private protected fixtures
remain distinct from route/console parity evidence.

The [Outer Wall Skeleton Apes](docs/outer-wall-skeleton-ape.md) now run selected
punches, carried/thrown barrels, bouncing fragments and staged deaths. Child
ownership uses slot/generation checks; optional actor pools have bounded fallbacks.
Full effects, whole-route and console parity remain incomplete.

The [early Short sword](docs/short-sword.md) now equips an ordinarily collected
Entrance item and runs selected original attacks in either hand. The family-37
view loads five palettes. Jewel/Stone sword special paths remain unavailable;
ordinary travel and Continue grant no replacement gear.

Selected normal-player [vertical room crossings](docs/early-castle-review.md) use
original early thresholds and upward/downward overlap adjustments. Horizontal
crossings keep priority. Camera-edge locks and transformation rules still require
parity work; a successful route is not proof of complete area or console parity.

The selected [Outer Wall Armor Lord](docs/outer-wall-armor-lord.md) now runs sword,
fire-wave, shield and staged death paths with checked actor/primitive ownership.
Its killed flag lasts for the loaded overlay, not forever in ordinary saves.
The copied player state distinguishes four-tick hand-weapon and subweapon startup:
Armor Lord observes the hand-weapon signal; Spear Guard observes both.
Contact retains element information and applies equipment element modifiers;
ailments, special damage reactions, elemental death presentation and original
contact priority remain incomplete. Death ribbons/lighting are bounded native
adaptations with explicit parity gaps. Protected component fixtures are not
uninterrupted-route or console evidence.

Selected [petrification](docs/player-petrification.md) now connects stone contact,
recoil/landing, directional escape, private palettes/wiggle and a selected lethal
completion to game over. Pause, weapons and save-room activation stay disabled.
Statue/dissolve children, rare variants, full lethal branches, fragments and
complete presentation/parity remain incomplete. The protected native fixture
injects hits; it is not evidence for ordinary Medusa encounters or full-game parity.

The selected [Outer Wall Medusa Heads](docs/outer-wall-medusa-heads.md) now use
private spawner profiles, bounded pools, original blue/gold flight and stone
contact. Gold heads reach the selected petrification service, whose statue and
complete death presentation remain incomplete. Native checks use protected room
starts and positioned contact, not ordinary traversal or console parity evidence.

The [Doppleganger arena](docs/doppleganger-arena.md), [runtime](docs/doppleganger-runtime.md)
and [entry/reward flow](docs/doppleganger-entry.md) now run selected native AI/body,
hand/knife/wing combat, physical defeat, placed entrance effects, block seals,
music and Life Max Up collection. Original Outer Wall boss overrides now select
BO4 before ordinary rooms; v7 saves retain all 32 original time-attack records and
visited bits. Versions 1–6 migrate explicitly; v4–6 Laboratory seconds convert to
event 6 HHMMSS. Continue starts supported save rooms at their center. BO4 has no
save rooms or separate public gameplay start. Mist/palette, high jump, stone,
complete elemental effects and whole-route/console parity remain incomplete.
Private protected room/route fixtures are not an unmodified-character playtest.

The [Long Library runtime](docs/library-runtime.md) now includes its own private
resource/table profile, original music, selected lamps/urns/jugs/busts, Flea/Flea
Armor, Dhuron, Thornweed/Corpseweed, Ectoplasm and Spellbook/Magic Tome encounters.
Bodies, owned children, beams, staged deaths and selected effects use Library
identities/tables. Re-entry replaces only matching owned children. Retained Flying
Zombie/body/blood adapters have explicit fixtures; original first-Library layouts
contain no ordinary Flying Zombie placements. Do not count those fixtures as route
coverage or reuse another overlay's raw IDs/tables.

[Lesser Demon](docs/library-lesser-demon.md) now has selected AI/attacks/summons,
physical defeat and [placed arena flow](docs/library-demon-entry.md): interpolated
camera lock, sealing books, music, defeat record and Life Max Up. Selected
[children](docs/library-demon-attacks.md) and [minions](docs/library-minions.md)
retain owned state and simulation-frame phase. The US initial bolt vertex has an
explicit safe-initialization concession. Protected/injected checks do not prove
whole-encounter or console parity.

The [Librarian introduction](docs/library-librarian.md) runs alignment/step49 hold,
chair, six initial voices and meeting/time record. [Buy/sell](docs/library-shop.md)
adds private catalogues/unlocks, quantities, paging, atomic equipment/gem/relic/map/
scroll transactions, speech, exit and returning greeting. v8 ordinary saves retain
learned-spell mask/order and game-clear unlock with explicit v1–v7 migrations.
Full detail/preview panels, tactics, enemy list, sound test, spell casting and
missing equipment effects remain incomplete. Chair gifts have injected-contact
checks; Gravity Boots/forms are separate. The [SEQ service](docs/library-sequence-audio.md)
keeps sequence and XA owners independent; console PCM/reverb parity remains unproven.

[Library furnishings](docs/library-furnishings.md) include the candle table/shadow,
permanent Stone Mask pickup, selected equipment metadata and private decoration
records. [Backgrounds](docs/library-backgrounds.md) include repeating models,
shadows, distant lighting grids, owned backdrop and foreground occlusion. The
[secret bookcase](docs/library-secret-bookcase.md) retains original push delay,
rotation/collision/lighting/subdivision and discovery flag 112. v8 stores discovery,
not transient pose/momentum. Keep all model/lighting/lookup data private. Original
ordering tables advance forward: higher priorities draw later; do not restore the
old descending renderer assumption. Stable native ties are not full GPU emulation.

[Curse](docs/character-status.md) and [poison](docs/player-poison.md) retain original
constitution-adjusted duration and independent transient timers. Curse blocks
equipped weapons/shields with original failure animation/sound; empty hands and
selected subweapons remain available. Poison halves new hand-attack power and
doubles ordinary incoming damage. Inventory pause freezes timers; room/stage
arrivals retain them; fresh ordinary Continue clears them. Full ailment visuals,
medicine and general elemental/damage-kind parity remain incomplete.

[Connected Library travel](docs/library-connections.md) now follows the original
Outer Wall connector in both directions. Teleport arrivals initialize actor-owned
red-door tiles before first player collision; direct development starts retain
their existing priming order. Missing destination resources preserve the previous
session/save store and show an error. Ordinary-character private checks cover
three crossings, stairs/doors, ownership and isolated save/Continue, with explicit
start/save-room transfer/discovery fixture. They are not whole-castle or console
replays. Sitting, Mist interaction, full shop services, full effects/audio and
complete Library/game progression remain incomplete. Do not call the area complete.

The [Royal Chapel foundation](docs/chapel-runtime.md) adds a stage-6 resource profile,
selected shared actors, Winged Guard/body pieces/spawner and Bat combat. Chapel
breakables/furnishings now use their own tables, with selected drops/debris. Chapel
Selected [Chapel mechanisms/scenery](docs/chapel-mechanisms.md) now add the shortcut
statue/block and saved flag 96, gargoyle states/tiles, walls and staircase shadow.
Selected [Chapel Crow/Raven/Skelerang combat](docs/chapel-enemies.md) now runs with
its own private tables, original bird phases/deaths and owned boomerangs. Shared
Skelerang re-entry protects unrelated children in both Chapel and Marble.
Selected [Chapel Bone Halberd combat](docs/chapel-halberd.md) now includes its owned
weapon, movement/spin/jump/lunge and nine-part death using private original tables.
Selected tower/scenery/Maria/boss travel now connects; further destinations and
whole-route parity remain incomplete. Private controlled fixtures are not console parity.

Selected [Chapel Corner Guard combat](docs/chapel-corner-guard.md) now includes
stair-bound movement, original directional attacks, owned weapon contact, selected
dust and private sprite-part death. Its remembered movement direction is separate
from sprite facing. Controlled component fixtures do not establish whole-route or
console parity; see the current connected progression scope above.

Selected [Chapel Bone Pillar skulls/fire](docs/chapel-bone-pillar.md) now run
side-dependent attacks, original fire/embers, delayed pieces and released upper
skull bounce/death. The [spike-ball variant](docs/chapel-spike-ball.md) now supports
intact destruction, release/bounce/roll, invulnerable contact and linked enemy damage.
Its owned outside-stage attacks share four original cooldown groups. Further
Chapel destinations and full encounter/console parity remain incomplete.

Selected [Chapel Hunting Girl combat](docs/chapel-hunting-girl.md) now runs the
spirit/sword, three combos and owned blade children, hit recovery and death.
Non-attacking motion omits null-trail writes while retaining END/cursor processing;
read the documented mandatory C adaptation before changing it. Its console
comparison and whole-route Chapel parity remain incomplete.

Selected [Chapel Spectral Sword combat](docs/chapel-spectral-sword.md) now runs
sixteen owned weapons, original readiness/orbits, movement and radius attack,
independent blade destruction, aura and parent death/cleanup. Retain the SDK
square-root quantization, screen-anchored movement target, integer-only ready
weapon position assignment and hidden dead children until parent cleanup.
Uninterrupted Chapel route/encounter and console parity remain incomplete.

Selected [Chapel spike-corridor behavior](docs/chapel-spikes.md) now includes its
owned damage actor, maximum-HP fraction, equipped Spike Breaker destruction and
original debris/dust. Keep ordinary defense separate from environmental HP loss.
Cleared spikes stay transient; v8 retains the equipped armor without a format change.
Further Chapel destinations, complete scripts and full damage-kind reaction parity
remain incomplete. Granted-equipment component checks do not establish ordinary-route parity.

Selected [Chapel bells](docs/chapel-bells.md) now run private spawners/profiles/models,
rope/body projection, original damped swing/ringing and inside control/protection.
Ordered draw-area primitives clip intervening draws around the player priority.
Keep their screen coordinates distinct from the room-owned anchor. Full GPU,
afterimage/transformed-player and whole-route parity remain incomplete; protected
component fixtures do not prove ordinary traversal.

Selected [Chapel tower exits](docs/chapel-tower-exits.md) now update original
horizontal windows after player movement and route through adjusted neighbor cells
in the same tick. Keep the moving reference origin separate from physical player/map
coordinates; its paired cell/X changes preserve world position. Destination actors
initialize before first draw/collision, with one audio synthesis frame per simulation
tick. Four protected explicit-start crossings are not all-door, ordinary-route or
console parity evidence. Selected connected travel now runs; further destinations remain gated.

Selected [Chapel sky backgrounds](docs/chapel-sky.md) now run the original private
cloud planes and repeating land strip. Preserve signed-X/unsigned-depth phase and
defined 16.16 wrap, private UV/model/grid/profile reads and SDK projection rounding.
Malformed empty rows fail explicitly instead of retaining the source's unbounded
scan. Controlled rendered contribution/phase checks are not whole-route or console
parity evidence; complete scenery/scripts and whole-route Chapel parity remain incomplete.

Selected [Chapel stained glass](docs/chapel-stained-glass.md) now runs private
panel poses/vertices/colors/subdivision, light fills/rays/glow and window/ceiling
backgrounds. Palette interpolation passes copied commands to owned renderer memory;
never mutate borrowed private inputs. Preserve documented US packed-coordinate
midpoints and reversed floor-color assignment pending console comparison. Checked
output cursors replace exhausted-pointer access. Protected component/render checks
are not ordinary-route or console parity; complete stage presentation remains unproven.

The internal [Chapel confessional](docs/chapel-confessional.md) now integrates chairs,
original sitting/sleep letters, priest/parishioner variants, curtains, attackable
blades, selected reward and private SEQ chime with XA restoration. Selected Maria,
Hippogryph and connected progression now run within their documented limits.
Controlled private fixtures are not uninterrupted-route or console parity evidence.

The internal [Chapel Maria event](docs/chapel-maria.md) now runs its placed script,
event-created controller/actor, private speech/portraits, original camera-anchor
pan and selected departure. Keep room coordinates distinct from screen scroll,
flag 99 distinct from other Maria/Entrance events, and pause/stopwatch gates separate.
Version-eight saves retain the flag. Transformation approaches, skip settings,
full presentation/timing parity and whole-route Chapel replay remain incomplete.
