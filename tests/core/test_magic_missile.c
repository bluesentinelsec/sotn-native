#include "actors_internal.h"
#include "character_fixture.h"
#include "player_internal.h"
#include <sotn/combat.h>
#include <sotn/magic_missile.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static bool Callbacks(void)
{
    SotnMagicMissile s = {.x = 32760 * 65536 + 123, .y = SDL_MIN_SINT32 + 789, .item = 25};
    SotnMagicMissileInput in = {.hand = 1, .priority = 509, .facing_left = true};
    SotnMagicMissileEvents e = {0};
    CHECK(SotnMagicMissile_Tick(&s, in, &e) && s.x == -32744 * 65536 + 123 &&
          s.y == 32762 * 65536 + 789 && s.item == 25 && s.bank == 0x8012 && s.texture == 0x66 &&
          s.palette == 0x128 && s.priority == 511 && s.flags == 0x140000 && e.animation == 1 &&
          e.sound == 0x669 && e.use_ticks == 4 && e.clear_shield && !e.properties && !e.factory);
    const SotnMagicMissile anchored = s;
    in.pose = 20;
    in.remaining = 2;
    CHECK(SotnMagicMissile_Tick(&s, in, &e) && e.factory && !e.sound && !e.animation &&
          !e.properties && !e.use_ticks && !SDL_memcmp(&s, &anchored, sizeof(s)));
    in.pose = 31;
    in.remaining = -1;
    CHECK(SotnMagicMissile_Tick(&s, in, &e) && s.phase == 2 && s.lifetime == 16 &&
          (s.draw_flags & 0x80) && !e.factory);
    for (Uint16 i = 0; i < 15; ++i)
        CHECK(SotnMagicMissile_Tick(&s, in, &e) && !s.removed);
    CHECK(SotnMagicMissile_Tick(&s, in, &e) && s.removed);
    s = (SotnMagicMissile){.x = SDL_MAX_SINT32 - 2, .y = 345};
    in = (SotnMagicMissileInput){
        .arrow = true,
        .priority = 148,
        .parent = {.exists = true, .bank = 7, .palette = 0x133, .texture = 90, .item = 0}};
    CHECK(SotnMagicMissile_Tick(&s, in, &e) && s.phase == 1 && s.x == SDL_MAX_SINT32 - 2 &&
          s.y == 345 && s.velocity_x == 5 * 65536 && s.bank == 7 && !s.item &&
          s.flags == 0x08100000 && e.properties && e.animation == 2 && !e.clear_shield &&
          !e.use_ticks);
    in.hit_flags = 3;
    CHECK(SotnMagicMissile_Tick(&s, in, &e) && s.phase == 1 &&
          s.x == SDL_MIN_SINT32 + 5 * 65536 - 3 && !e.properties && !e.animation);
    in.hit_flags = 2;
    CHECK(SotnMagicMissile_Tick(&s, in, &e) && s.phase == 1 &&
          s.x == SDL_MIN_SINT32 + 10 * 65536 - 3);
    const Sint32 x = s.x;
    in.hit_flags = 1;
    CHECK(SotnMagicMissile_Tick(&s, in, &e) && s.phase == 2 && s.x == x && e.animation == 3 &&
          !e.properties);
    in.remaining = -1;
    CHECK(SotnMagicMissile_Tick(&s, in, &e) && s.removed);
    s = (SotnMagicMissile){.x = 123};
    in.parent.exists = false;
    in.remaining = 0;
    CHECK(SotnMagicMissile_Tick(&s, in, &e) && s.removed && !e.properties && !e.animation &&
          s.x == 123);
    s = (SotnMagicMissile){.x = 123};
    const SotnMagicMissile before = s;
    const SotnMagicMissileEvents events = e;
    in.priority = 510;
    CHECK(!SotnMagicMissile_Tick(&s, in, &e) && !SDL_memcmp(&s, &before, sizeof(s)) &&
          !SDL_memcmp(&e, &events, sizeof(e)));
    return true;
}
static void FillAnimations(Uint8 *overlay)
{
    // Authored timelines; no original private animation records are compiled in.
    for (Uint16 i = 0; i < 33; ++i)
    {
        Put16(overlay + 0x77c + i * 4U, i == 32 ? 0xffff : 2);
        Put16(overlay + 0x77e + i * 4U, (Uint16)(i + 1));
    }
    for (Uint16 i = 0; i < 7; ++i)
    {
        Put16(overlay + 0x760 + i * 4U, i == 6 ? 0xffff : 2);
        Put16(overlay + 0x762 + i * 4U, (Uint16)(i + 1));
    }
    Put16(overlay + 0x800, 3);
    Put16(overlay + 0x802, 77);
}
static bool Animation(void)
{
    Uint8 overlay[0x808] = {0};
    FillAnimations(overlay);
    const SotnDataView image = {overlay, sizeof(overlay)};
    SotnMagicMissileAnimation a = {0};
    CHECK(SotnMagicMissile_Animate(&a, image, 1) && a.pose == 0 && a.remaining == 2 &&
          a.frame == 1);
    for (Uint16 i = 0; i < 70; ++i)
        CHECK(SotnMagicMissile_Animate(&a, image, 1));
    CHECK(a.pose == 31 && a.remaining == -1 && a.frame == 32);
    a = (SotnMagicMissileAnimation){0};
    for (Uint16 i = 0; i < 100; ++i)
        CHECK(SotnMagicMissile_Animate(&a, image, 2) && a.pose == 0 && a.remaining > 0 &&
              a.frame == 77);
    const SotnMagicMissileAnimation before = a;
    CHECK(!SotnMagicMissile_Animate(&a, (SotnDataView){0}, 2) &&
          !SDL_memcmp(&a, &before, sizeof(a)));
    a = (SotnMagicMissileAnimation){0};
    for (Uint16 i = 0; i < 20; ++i)
        CHECK(SotnMagicMissile_Animate(&a, image, 3));
    CHECK(a.pose == 5 && a.remaining == -1 && a.frame == 6);
    return true;
}
static void FillCharacter(Uint8 *dra)
{
    TestCharacter_Fill(dra);
    dra[0x500 + 18] = 0x24;
    Put16(dra + 0x500 + 40, 2);
    Uint8 *record = dra + 0x500 + 25U * 52U;
    Put16(record + 8, 40);
    record[14] = 8;
    record[15] = 22;
    record[17] = 130;
    record[18] = 0x60;
    record[19] = 50;
    record[22] = 4;
    record[25] = 1;
    record[26] = 9;
    Put16(record + 40, 2);
    Put16(record + 48, 4);
}
static bool MetadataAndShield(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnCharacter *character = NULL;
    CHECK(dra);
    FillCharacter(dra);
    SotnEquipment e = {0};
    SotnItem item = {0};
    const SotnDataView image = {dra, 0x11000};
    // Ordinary metadata validates the exact private chain/category/hit contract.
    CHECK(SotnEquipment_ReadMagicMissile(image, &e) && e.id == 25 && e.chain_limit == 4 &&
          e.family == 22 && e.hit_type == 2 && e.invincibility == 9 &&
          SotnEquipment_Read(image, 25, &e) && SotnItem_Read(image, false, 25, &item) &&
          item.supported && SotnCharacter_Create(image, &character));
    const SotnEquipment valid = e;
    dra[0x500 + 25U * 52U + 22] = 1;
    CHECK(!SotnEquipment_Read(image, 25, &e) && !SDL_memcmp(&e, &valid, sizeof(e)));
    dra[0x500 + 25U * 52U + 22] = 4;
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(character, &before, NULL));
    Uint32 random = 0xabcdeU, expected = random;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnCharacter_MagicMissileProperties(character, 25, false, &random, &e) &&
          random == expected && e.id == 25 && e.attack >= 40 && e.hit_type == 2 &&
          SotnCharacter_Get(character, &after, NULL) &&
          !SDL_memcmp(&before, &after, sizeof(before)));
    const SotnEquipment previous = e;
    CHECK(!SotnCharacter_MagicMissileProperties(character, 71, false, &random, &e) &&
          random == expected && !SDL_memcmp(&previous, &e, sizeof(e)));
    CHECK(SotnCharacter_MagicMissileProperties(character, 0, false, &random, &e) && !e.id);
    SotnCharacter_Destroy(character);
    SDL_free(dra);

    SotnPlayer p = {.attack_mode = 0x8001,
                    .buttons = SOTN_BUTTON_RIGHT_HAND,
                    .vx = 12345,
                    .attack = {.active = true, .equipment = 86, .sprite = 3}};
    p.subweapon_ticks = 7;
    p.spell_use_ticks = 9;
    const SotnPlayer absent = p;
    CHECK(SotnPlayer_ClearShield(&p) && !SDL_memcmp(&p, &absent, sizeof(p)));
    for (Uint16 phase = 1; phase <= 3; ++phase)
    {
        p.shield_cursor = (SotnShield){.phase = phase, .remaining = 5};
        p.shield = (SotnPlayerAttack){.active = true, .equipment = 16, .sprite = 7};
        CHECK(SotnPlayer_ClearShield(&p) && !SDL_memcmp(&p, &absent, sizeof(p)));
    }
    p.shield_cursor.phase = 4;
    const SotnPlayer malformed = p;
    CHECK(!SotnPlayer_ClearShield(&p) && !SDL_memcmp(&p, &malformed, sizeof(p)) &&
          !SotnPlayer_ClearShield(NULL));
    return true;
}
static bool Shared(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(1, 23U * 0x7000U);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *before = SDL_malloc(sizeof(*before));
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r && before);
    FillCharacter(dra);
    Uint8 *overlay = weapons + 22U * 0x7000U + 0x4000;
    FillAnimations(overlay);
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *bp = dra + 0xd1d4 + (i ? 56U : 50U) * 6U;
        bp[0] = (Uint8)i;
        bp[1] = bp[2] = 1;
        bp[4] = i ? 6 : 0x26;
    }
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b8 + 13] = 31;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    actors->primitives[7].owner = 777;
    SotnBatRuntimeInput in = {
        .dra = actors->dra,
        .weapons = {weapons, 23U * 0x7000U},
        .weapon_family = {22, 22},
        .weapon_bound = {true, true},
        .player = {.hp = 70, .x = 128 * 65536, .y = 140 * 65536, .priority = 148}};
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots));
    bool allocated = false;
    CHECK(SotnBatRuntime_RequestMagicMissile(r, (SotnPlayerFactorySource){.priority = 148},
                                             0x802032, &allocated) &&
          allocated);
    SotnCharacterState initial = {0}, after = {0};
    CHECK(SotnCharacter_Get(character, &initial, NULL));
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->children[1].magic &&
          r->children[1].kind == 0xf0 && r->children[1].parameter == 0x8000 &&
          r->children[1].state.magic.motion.item == 25 && !r->children[1].primitive_count &&
          r->magic_clear_shield && r->use_ticks == 4 && !r->spell_use_ticks && !r->magic_mask &&
          SotnActors_CommitBatPrimitives(actors, r));
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == initial.random &&
          actors->primitives[7].owner == 777);
    *before = *r;
    in.freeze = 3;
    CHECK(SotnBatRuntime_Tick(r, character, in) && !r->magic_clear_shield && !r->use_ticks &&
          !SDL_memcmp(&before->children[1], &r->children[1], sizeof(r->children[1])));
    in.freeze = 0;
    // Occupied real roots omit the arrow and its sound, without a replacement cooldown or RNG.
    for (Uint16 i = 0; i < 8; ++i)
        r->factories[i].cursor = (SotnPlayerFactory){.entity_id = 1,
                                                     .blueprint = 50,
                                                     .initialized = true,
                                                     .amount = 1,
                                                     .per_cycle = 1,
                                                     .kind = 6,
                                                     .origin = 2,
                                                     .delay = 20,
                                                     .source.priority = 148};
    r->children[1].state.magic.animation.pose = 20;
    r->children[1].state.magic.animation.remaining = 2;
    CHECK(SotnBatRuntime_Tick(r, character, in) && !r->children[2].kind && !r->sound_count &&
          SotnCharacter_Get(character, &after, NULL) && after.random == initial.random);
    SDL_zero(r->factories);
    r->children[1].state.magic.animation.remaining = 2;
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->factories[0].magic_factory &&
          r->factories[0].magic_parent_slot == 17 && !r->children[2].kind && r->sound_count == 1 &&
          r->sounds[0] == 0x625);
    // The arrow reads the actual replacement parent slot, without a generation restriction.
    r->children[1].generation = ++r->serial;
    const Uint32 generation = r->children[1].generation;
    Put16(overlay + 0x800, 0xffff);
    *before = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(before, r, sizeof(*r)) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == initial.random &&
          actors->primitives[7].owner == 777);
    Put16(overlay + 0x800, 3);
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->children[1].generation == generation &&
          r->children[2].magic && r->children[2].kind == 0xf1 && r->magic_mask == 4 &&
          r->children[2].state.magic.motion.item == 25 &&
          r->children[2].state.magic.motion.bank == 0x8012 &&
          r->children[2].state.magic.attack.right_hand &&
          r->children[2].state.magic.attack.hit_type == 2 &&
          r->children[2].state.magic.animation.frame == 77 && !r->children[2].parameter &&
          SotnActors_CommitBatPrimitives(actors, r));
    // Both original56 arrows keep parameter0, even F1; chain counting must use that raw word.
    Uint32 expected = initial.random;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    // Only the initialized PLAYER_LOCKED bow follows actual post-stage shove words.
    // The CAMERA_LOCKED arrow keeps its own origin, with no callback or RNG advance.
    const SotnBatRuntimeChild bow = r->children[1], arrow = r->children[2];
    r->children[1].state.magic.timeline = 0;
    *before = *r;
    CHECK(!SotnBatRuntime_PlayerShove(r, 123, -456) && !SDL_memcmp(before, r, sizeof(*r)));
    r->children[1] = bow;
    CHECK(SotnBatRuntime_PlayerShove(r, SDL_MAX_SINT32, SDL_MIN_SINT32));
    Uint32 bx = (Uint32)bow.source.x + (Uint32)SDL_MAX_SINT32,
           by = (Uint32)bow.source.y + (Uint32)SDL_MIN_SINT32;
    Sint32 sx = 0, sy = 0;
    SDL_memcpy(&sx, &bx, sizeof(sx));
    SDL_memcpy(&sy, &by, sizeof(sy));
    CHECK(r->children[1].source.x == sx && r->children[1].source.y == sy &&
          r->children[1].state.magic.motion.x == sx && r->children[1].state.magic.motion.y == sy &&
          !SDL_memcmp(&arrow, &r->children[2], sizeof(arrow)));
    r->children[1] = bow;
    const Sint32 x = r->children[2].source.x;
    CHECK(SotnBatRuntime_Hit(r, 2, r->children[2].generation, 3) &&
          SotnBatRuntime_Tick(r, character, in) && r->children[2].state.magic.motion.phase == 1 &&
          r->children[2].source.x == x + 5 * 65536);
    CHECK(SotnBatRuntime_Hit(r, 2, r->children[2].generation, 1) &&
          SotnBatRuntime_Tick(r, character, in) && r->children[2].state.magic.motion.phase == 2 &&
          r->magic_mask == 4);
    for (Uint16 i = 0; i < 20 && r->children[2].kind; ++i)
        CHECK(SotnBatRuntime_Tick(r, character, in));
    CHECK(!r->children[2].kind && !r->magic_mask && SotnBatRuntime_PruneMagicMissile(r) &&
          SotnBatRuntime_PruneRoomFactories(r) && SotnActors_CommitBatPrimitives(actors, r) &&
          !r->children[1].kind && SotnCharacter_Get(character, &after, NULL) &&
          after.random == expected && actors->primitives[7].owner == 777);
    // Real culling precedes animation, so an offscreen bow never reads this bad timeline.
    in.player.x = 600 * 65536;
    Put16(overlay + 0x77c, 0xffff);
    CHECK(SotnBatRuntime_RequestMagicMissile(r, (SotnPlayerFactorySource){.priority = 148}, 0x1032,
                                             &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) && !r->children[1].kind &&
          !r->magic_mask && SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    SotnCharacter_Destroy(character);
    SDL_free(before);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
int main(void)
{
    return Callbacks() && Animation() && MetadataAndShield() && Shared() ? 0 : 1;
}
