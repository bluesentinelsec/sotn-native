#include "player_internal.h"
#include <sotn/death_image.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x11000], tiles[32 * 16 * 2], types[2];
    SotnDeathImage image;
} Fixture;
static void P32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (8U * i));
}
static void P16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Init(Fixture *f)
{
    SDL_memset(f, 0, sizeof(*f));
    P32(f->dra + 0xc0, 0x800a1000);
    P32(f->dra + 0xc4, 0x800a4000);
    f->dra[0x100] = 'A' - 32;
    f->dra[0x101] = f->dra[0x180] = 255;
    for (Uint16 body = 0; body < 2; ++body)
        for (Uint16 id = 0; id < (body ? 90 : 169); ++id)
        {
            Uint8 *p = f->dra + (body ? 0x4000 + id * 32U : 0x1000 + id * 52U);
            P32(p, 0x800a0100);
            P32(p + 4, 0x800a0180);
            if (body)
                p[28] = (Uint8)(id == 26 ? 0 : id == 48 ? 2 : (id == 57 || id == 60) ? 3 : 1);
        }
    for (Uint16 id = 0; id < 247; ++id)
        P32(f->dra + 0x101b8 + id * 4U, 0x800a0700);
    for (Uint16 i = 0; i < 16; ++i)
        P32(f->dra + 0x700 + i * 4U, 3U | ((Uint32)i + 1U) << 16);
    P32(f->dra + 0x740, 65535);
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            f->tiles[(y * 32 + x) * 2] = 1;
}
static bool Counters(void)
{
    SotnStatBuffs s = {0};
    Uint16 mask = 65535;
    CHECK(SotnStatBuffs_Mask(&s, &mask) && !mask);
    const Uint16 durations[] = {0, 1, 2, 511, 512, 4096};
    for (Uint16 id = 0; id < 16; ++id)
        for (Uint16 k = 0; k < 6; ++k)
        {
            CHECK(SotnStatBuffs_Reset(&s));
            s.timers[id] = durations[k];
            CHECK(SotnStatBuffs_Tick(&s));
            const Sint32 expected = durations[k] != 0 ? (Sint32)durations[k] - 1 : durations[k];
            CHECK(s.timers[id] == expected && SotnStatBuffs_Mask(&s, &mask) &&
                  mask == (expected ? (Uint16)(1U << id) : 0));
        }
    CHECK(SotnStatBuffs_Reset(&s) && SotnStatBuffs_Minimum(&s, 0, 512) && s.timers[0] == 512 &&
          SotnStatBuffs_Tick(&s) && s.timers[0] == 511 && SotnStatBuffs_Give(&s, 0) &&
          SotnStatBuffs_Minimum(&s, 0, 512) && s.timers[0] == 4096);
    const SotnStatBuffs before = s;
    CHECK(!SotnStatBuffs_Minimum(&s, 0, 4097) && !SotnStatBuffs_Give(&s, 16) &&
          !SotnStatBuffs_Clear(&s, 16) && !SDL_memcmp(&s, &before, sizeof(s)));
    s.timers[15] = -1;
    const SotnStatBuffs invalid = s;
    mask = 0x1234;
    CHECK(!SotnStatBuffs_Mask(&s, &mask) && mask == 0x1234 && !SotnStatBuffs_Tick(&s) &&
          !SDL_memcmp(&s, &invalid, sizeof(s)));
    CHECK(SotnStatBuffs_Clear(&s, 15) && SotnStatBuffs_Mask(&s, NULL) && SotnStatBuffs_Reset(&s) &&
          SotnStatBuffs_Mask(&s, &mask) && !mask);
    return true;
}
static bool Character(Fixture *f)
{
    Init(f);
    Uint8 *armor = f->dra + 0x4000 + 22 * 32U;
    P16(armor + 10, 0xfffb);
    SotnCharacter *c = NULL;
    SotnCharacterState state = {0}, after = {0};
    SotnCharacterStats a = {0};
    CHECK(SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &c) &&
          SotnCharacter_Collect(c, true, 22) && SotnCharacter_Equip(c, SOTN_EQUIP_ARMOR, 22) &&
          SotnCharacter_Get(c, &state, &a) && !a.defense);
    CHECK(SotnCharacter_SetStatBuffMask(c, 1) && SotnCharacter_Get(c, &after, &a) &&
          a.defense == 15 && !SDL_memcmp(&state, &after, sizeof(state)));
    P16(armor + 10, 1000);
    CHECK(SotnCharacter_Restore(c, &state) && SotnCharacter_Get(c, NULL, &a) && a.defense == 999);
    P16(armor + 10, 32760);
    CHECK(SotnCharacter_Restore(c, &state) && SotnCharacter_Get(c, NULL, &a) && !a.defense);
    P16(armor + 10, 0);
    armor[13] = 99;
    state.base[1] = 49;
    CHECK(SotnCharacter_Restore(c, &state) && SotnCharacter_Get(c, NULL, &a) && a.total[1] == 841 &&
          a.defense == 46);
    armor[13] = 0;
    state.base[1] = 6;
    P16(armor + 10, 0xfffb);
    armor[12] = armor[14] = armor[15] = 90;
    CHECK(SotnCharacter_Restore(c, &state) && SotnCharacter_SetStatBuffMask(c, 31) &&
          SotnCharacter_Get(c, &after, &a) && a.bonus[0] == 99 && a.bonus[2] == 99 &&
          a.bonus[3] == 99 && a.total[0] == 105 && a.total[1] == 6 && a.total[2] == 402 &&
          a.total[3] == 105 && a.attack[0] == 125 && a.unarmed_attack == 125 && a.defense == 15 &&
          !SDL_memcmp(&state, &after, sizeof(state)));
    CHECK(SotnCharacter_SetStatBuffMask(c, 0xfe0) && SotnCharacter_Get(c, NULL, &a) &&
          a.resistance == 0xf900 && a.immunity == 0x200 && !(a.resistance & 0x200));
    CHECK(SotnCharacter_SetStatBuffMask(c, 0xf000) && SotnCharacter_Get(c, NULL, &a) &&
          a.total[0] == 96 && a.total[2] == 366 && a.total[3] == 96 && !a.defense &&
          !a.resistance && !a.immunity);
    const SotnCharacterStats prior = a;
    armor[28] = 2;
    CHECK(!SotnCharacter_SetStatBuffMask(c, 0) && SotnCharacter_Get(c, &after, &a) &&
          !SDL_memcmp(&a, &prior, sizeof(a)) && !SDL_memcmp(&state, &after, sizeof(state)));
    SotnCharacter_Destroy(c);
    return true;
}
static bool Capture(void *ctx, Uint32 gen)
{
    Fixture *f = ctx;
    Uint8 b[5120] = {0};
    (void)gen;
    return SotnDeathImage_Begin(&f->image, (SotnDataView){b, sizeof(b)});
}
static bool Decay(void *ctx, Uint32 gen, bool *done)
{
    Fixture *f = ctx;
    (void)gen;
    if (!SotnDeathImage_Tick(&f->image))
        return false;
    *done = !f->image.remaining;
    return true;
}
static bool Fury(Fixture *f)
{
    Init(f);
    SotnCharacter *c = NULL;
    SotnPlayer *p = NULL;
    SotnCharacterState state = {0};
    SotnCharacterStats stats = {0};
    SotnStatBuffs buffs = {0};
    CHECK(SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &c) &&
          SotnCharacter_Collect(c, true, 22) && SotnCharacter_Equip(c, SOTN_EQUIP_ARMOR, 22) &&
          SotnCharacter_Get(c, &state, &stats));
    state.hp = 30;
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)},
                            (SotnCollisionMap){
                                {f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16},
                            100, 135, &p) &&
          SotnPlayer_SetCharacter(p, &state, stats) &&
          SotnPlayer_SetDeathCapture(p, Capture, Decay, f) && !SotnPlayer_SetDamageContext(p, 1) &&
          SotnPlayer_SetStatBuffContext(p, &buffs, stats) && SotnPlayer_SetDamageContext(p, 1) &&
          SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}));
    p->fixed_stage = true;
    CHECK(SotnPlayer_Tick(p, 0) && p->hp == 30 && p->defense_buff_minimum == 512 &&
          !buffs.timers[0] && SotnStatBuffs_Minimum(&buffs, 0, p->defense_buff_minimum) &&
          SotnStatBuffs_Tick(&buffs) && buffs.timers[0] == 511 &&
          SotnCharacter_SetStatBuffMask(c, 1) && SotnCharacter_Get(c, NULL, &stats) &&
          stats.defense == 20);
    CHECK(SotnPlayer_SetStatBuffContext(p, &buffs, stats) &&
          SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 30, .parameters = 1}) &&
          SotnPlayer_SetDamageContext(p, 10) && SotnPlayer_Tick(p, 0) && p->hp == 20 &&
          p->defense_buff_minimum == 512 && buffs.timers[0] == 511);
    p->step = SOTN_PLAYER_STAND;
    p->substep = 0;
    p->invincibility = 0;
    p->animation.id = 247;
    CHECK(SotnPlayer_SetDamageContext(p, 20));
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(p, &before, sizeof(*p)) && buffs.timers[0] == 511);
    p->animation.id = 1;
    p->hp = 1;
    CHECK(SotnPlayer_SetDamageContext(p, 20) && SotnPlayer_Tick(p, 0) && p->water_death.started &&
          !p->hp && !p->defense_buff_minimum && buffs.timers[0] == 511);
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, true) && !p->defense_buff_minimum);
    SotnPlayer_Close(p);
    SotnCharacter_Destroy(c);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Counters() && Character(f) && Fury(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
