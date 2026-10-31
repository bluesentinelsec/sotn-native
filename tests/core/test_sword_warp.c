#include "character_fixture.h"
#include "player_internal.h"
#include <sotn/bat_runtime.h>
#include <sotn/combat.h>

#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x11000], weapons[0x1a0000], tiles[64 * 16 * 2], types[2];
    SotnPlayer *p;
    SotnCharacter *character;
    SotnBatRuntime runtime;
    SotnPrimitiveSlots slots;
    SotnCollisionMap map;
    Uint16 spent;
} Fixture;
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    TestCharacter_Put32(p, value);
}
static bool Factory(void *context, SotnPlayerFactorySource source, Uint32 packed, Sint16 argument,
                    bool *allocated)
{
    Fixture *f = context;
    return SotnBatRuntime_RequestSword(&f->runtime, source, packed, argument, allocated);
}
static bool Create(Fixture *f, bool right)
{
    SotnPlayer_Close(f->p);
    SotnCharacter_Destroy(f->character);
    SDL_memset(f, 0, sizeof(*f));
    TestCharacter_Fill(f->dra);
    f->dra[0x500 + 18] = 0x24;
    Uint8 *e = f->dra + 0x500 + 123 * 52;
    Put16(e + 8, 12);
    e[15] = 43;
    e[17] = 1;
    e[18] = 0x41;
    e[22] = 128;
    e[24] = 197;
    Put16(e + 40, 2);
    Put16(e + 48, 5);
    e[26] = 8;
    e = f->dra + 0x500 + 197 * 52;
    Put16(e + 8, 27);
    e[15] = 43;
    e[17] = 135;
    e[18] = 0xd9;
    e[19] = 54;
    e[22] = 128;
    Put16(e + 36, 5);
    Put16(e + 40, 2);
    Put16(e + 48, 7);
    e[26] = 8;
    for (Uint16 i = 0; i < 247; ++i)
        Put32(f->dra + 0x101b8 + i * 4U, 0x800a7000);
    for (Uint16 i = 0; i < 20; ++i)
        Put32(f->dra + 0x7000 + i * 4U, 0x00010002);
    Put32(f->dra + 0x7050, 0xffff);
    Put32(f->dra + 0x101b8 + 0xd9 * 4U, 0x800a7100);
    for (Uint16 i = 0; i < 20; ++i)
        Put32(f->dra + 0x7100 + i * 4U, 0x00010002);
    Put32(f->dra + 0x7150, 0xffff);
    Uint8 *b = f->dra + 0xd1d4 + 54 * 6;
    b[0] = 1;
    b[1] = b[2] = 1;
    b[4] = 0x23;
    b = f->dra + 0xd1d4 + 61 * 6;
    b[0] = 31;
    b[1] = b[2] = 1;
    b[4] = 0x21;
    b = f->dra + 0xd1d4 + 62 * 6;
    b[0] = 2;
    b[1] = b[2] = 1;
    b[4] = 6;
    f->dra[0xd4b8 + 2] = 32;
    f->dra[0xd4b8 + 3] = 47;
    f->dra[0xd4b8 + 6] = f->dra[0xd4b8 + 7] = 16;
    f->dra[0xd4b8 + 12] = 17;
    f->dra[0xd4b8 + 13] = 31;
    for (Uint16 p = 20; p <= 21; ++p)
    {
        Uint8 *r = f->dra + 0xd9b8 + p * 10U;
        Put16(r, 90);
        Put16(r + 2, 60);
        Put16(r + 4, 30);
        Put16(r + 6, 0x100);
        Put16(r + 8, 49);
    }
    Uint8 *overlay = f->weapons + 43 * 0x7000 + 0x4000;
    for (Uint16 i = 0; i < 20; ++i)
        Put32(overlay + 0xf40 + i * 4U, 1U | ((Uint32)(i % 3U ? 0x100U + i : 0x24U) << 16));
    Put32(overlay + 0xf90, 0xffff);
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 64; ++x)
            f->tiles[(y * 64U + x) * 2] = 1;
    f->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 64, 16};
    CHECK(SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &f->character));
    CHECK(SotnCharacter_Collect(f->character, false, 123));
    CHECK(SotnCharacter_Equip(f->character, right ? SOTN_EQUIP_RIGHT : SOTN_EQUIP_LEFT, 123));
    CHECK(SotnCharacter_Equip(f->character, right ? SOTN_EQUIP_LEFT : SOTN_EQUIP_RIGHT, 0));
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, f->map, 256, 135, &f->p));
    CHECK(SotnPlayer_SetEquipment(f->p, (SotnDataView){f->weapons, sizeof(f->weapons)},
                                  right ? 0 : 123, right ? 123 : 0));
    return true;
}
static bool Tick(Fixture *f, Uint16 buttons)
{
    SotnCharacterState progress = {0};
    SotnCharacterStats stats = {0};
    SotnPlayerState p = {0};
    CHECK(SotnCharacter_Get(f->character, &progress, &stats) &&
          SotnPlayer_SetCharacter(f->p, &progress, stats));
    CHECK(SotnPlayer_SetSwordContext(f->p, Factory, f, f->runtime.children[0].kind != 0));
    CHECK(SotnPlayer_Tick(f->p, buttons) && SotnPlayer_GetState(f->p, &p));
    CHECK(SotnCharacter_SpendMP(f->character, p.mp_spent));
    f->spent = (Uint16)(f->spent + p.mp_spent);
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character,
                              (SotnBatRuntimeInput){.dra = {f->dra, sizeof(f->dra)},
                                                    .weapons = {f->weapons, sizeof(f->weapons)},
                                                    .primitive_slots = f->slots,
                                                    .player = p,
                                                    .image = {12, 24, -6, -12}}));
    return true;
}
static bool CommandAndMotion(void)
{
    SotnWeaponCommand c = {0};
    SotnWeaponCommand_Tick(&c, 0x4000, false, false);
    CHECK(c.phase == 1 && c.timer == 20);
    SotnWeaponCommand_Tick(&c, 0x6000, false, false);
    CHECK(c.phase == 2);
    SotnWeaponCommand_Tick(&c, 0x2080, false, false);
    CHECK(c.phase == 255 && c.timer == 20);
    for (Uint16 i = 0; i < 19; ++i)
        SotnWeaponCommand_Tick(&c, 0, true, false);
    CHECK(c.phase == 255 && c.timer == 1);
    SotnWeaponCommand_Tick(&c, 0, true, false);
    CHECK(c.phase == 0 && c.facing_left);
    SotnWeaponCommand_Tick(&c, 0x4000, true, false);
    SotnWeaponCommand_Tick(&c, 0xc000, false, false);
    SotnWeaponCommand_Tick(&c, 0x8000, false, false);
    CHECK(c.phase == 255);
    SotnWeaponCommand_Tick(&c, 0, false, true);
    CHECK(c.phase == 0);
    SotnSwordWarp s = {.x = SDL_MAX_SINT32 - 10};
    SotnSwordWarpEvents e = {0};
    CHECK(SotnSwordWarp_Tick(&s, 0, 2, 3, &e) && s.x == SDL_MAX_SINT32 - 10 &&
          s.velocity_x == 851968);
    CHECK(SotnSwordWarp_Tick(&s, 5, 1, 4, &e) && e.strike && s.phase == 2 && s.facing_left &&
          s.x < 0);
    const Sint32 x = s.x;
    CHECK(SotnSwordWarp_Tick(&s, 14, 1, 5, &e) && s.x == x && s.phase == 3 && !e.strike);
    CHECK(SotnSwordWarp_Tick(&s, 19, 1, 6, &e) && s.removed && s.blend == 0x10);
    const SotnSwordWarp before = s;
    CHECK(!SotnSwordWarp_Tick(&s, 19, 1, 6, &e) && !SDL_memcmp(&s, &before, sizeof(s)));
    return true;
}
static bool Casting(Fixture *f, bool right)
{
    CHECK(Create(f, right));
    CHECK(Tick(f, 0));
    CHECK(Tick(f, SOTN_BUTTON_DOWN));
    CHECK(Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT));
    CHECK(Tick(f, SOTN_BUTTON_RIGHT | (right ? SOTN_BUTTON_RIGHT_HAND : SOTN_BUTTON_LEFT_HAND)));
    CHECK(f->p->step == 15 && f->p->palette == 0x810d && f->spent == 5 && f->p->mp == 15 &&
          f->p->sound == 0x6f0);
    CHECK(f->runtime.children[0].kind == (right ? 0xf1 : 0xe1) &&
          f->runtime.children[0].initialized);
    const Uint32 identity = f->runtime.children[0].generation;
    Uint16 image_ticks = 0, strikes = 0, attacking = 0, recovery = 0;
    for (Uint16 t = 0; t < 100 && f->p->step == 15; ++t)
    {
        const SotnBatRuntime before = f->runtime;
        if (t == 2)
        {
            CHECK(SotnBatRuntime_Tick(
                &f->runtime, f->character,
                (SotnBatRuntimeInput){.dra = {f->dra, sizeof(f->dra)},
                                      .weapons = {f->weapons, sizeof(f->weapons)},
                                      .entering = true}));
            CHECK(!SDL_memcmp(&before, &f->runtime, sizeof(before)));
            const Sint32 recovery_before = f->p->sword_recovery;
            const SotnPlayerSimulatedPad pad = f->p->simulated_pad;
            CHECK(SotnPlayer_TickSpellFreeze(f->p, SOTN_BUTTON_JUMP, false));
            CHECK(f->p->sword_recovery == recovery_before &&
                  !SDL_memcmp(&pad, &f->p->simulated_pad, sizeof(pad)));
            SotnPlayerState frozen = {0};
            CHECK(SotnPlayer_GetState(f->p, &frozen));
            CHECK(SotnBatRuntime_Tick(&f->runtime, f->character,
                                      (SotnBatRuntimeInput){.dra = {f->dra, sizeof(f->dra)},
                                                            .player = frozen,
                                                            .freeze = 3,
                                                            .image = {12, 24, -6, -12}}));
            CHECK(!SDL_memcmp(&before.children[0], &f->runtime.children[0],
                              sizeof(before.children[0])));
        }
        if (t == 4)
        {
            CHECK(SotnCharacter_Equip(f->character, right ? 1 : 0, 0));
            CHECK(SotnPlayer_SetEquipment(f->p, (SotnDataView){f->weapons, sizeof(f->weapons)}, 0,
                                          0));
        }
        bool strike_pending = false;
        for (Uint16 i = 0; i < 8; ++i)
            strike_pending = strike_pending || (f->runtime.factories[i].cursor.entity_id &&
                                                f->runtime.factories[i].sword_parent);
        if (strike_pending)
        {
            SotnCharacterState character = {0}, after = {0};
            SotnPlayerState p = {0};
            CHECK(SotnCharacter_Get(f->character, &character, NULL) &&
                  SotnPlayer_GetState(f->p, &p));
            SotnBatRuntime *checkpoint = SDL_malloc(sizeof(*checkpoint));
            CHECK(checkpoint);
            *checkpoint = f->runtime;
            f->dra[0xd4b8 + 12] = 32;
            f->dra[0xd4b8 + 13] = 47;
            const bool rejected =
                !SotnBatRuntime_Tick(
                    &f->runtime, f->character,
                    (SotnBatRuntimeInput){.dra = {f->dra, sizeof(f->dra)},
                                          .weapons = {f->weapons, sizeof(f->weapons)},
                                          .player = p,
                                          .image = {12, 24, -6, -12}}) &&
                !SDL_memcmp(checkpoint, &f->runtime, sizeof(*checkpoint));
            SDL_free(checkpoint);
            f->dra[0xd4b8 + 12] = 17;
            f->dra[0xd4b8 + 13] = 31;
            CHECK(rejected && SotnCharacter_Get(f->character, &after, NULL) &&
                  !SDL_memcmp(&character, &after, sizeof(character)));
        }
        CHECK(Tick(f, 0));
        const SotnPlayerBlink *effect = NULL;
        const SotnPlayerFactory *pending = NULL;
        CHECK(SotnBatRuntime_Context(&f->runtime, identity, &effect, &pending) && !effect &&
              !pending);
        if (f->runtime.children[0].kind)
        {
            ++image_ticks;
            CHECK(f->runtime.children[0].generation == identity);
        }
        for (Uint16 i = 1; i < 16; ++i)
            if (f->runtime.children[i].kind == 0xe2 || f->runtime.children[i].kind == 0xf2)
            {
                ++strikes;
                CHECK(f->runtime.children[i].state.sword.parent_generation == identity &&
                      f->runtime.children[i].group == 0);
                attacking += f->runtime.children[i].state.sword.attack.active;
                if (strikes == 1)
                {
                    const SotnBatRuntime working = f->runtime;
                    SotnCharacterState character = {0}, after = {0};
                    SotnPlayerState p = {0};
                    CHECK(SotnCharacter_Get(f->character, &character, NULL) &&
                          SotnPlayer_GetState(f->p, &p));
                    CHECK(
                        !SotnBatRuntime_Tick(&f->runtime, f->character,
                                             (SotnBatRuntimeInput){.dra = {f->dra, sizeof(f->dra)},
                                                                   .player = p,
                                                                   .image = {12, 24, -6, -12}}));
                    CHECK(!SDL_memcmp(&working, &f->runtime, sizeof(working)) &&
                          SotnCharacter_Get(f->character, &after, NULL) &&
                          !SDL_memcmp(&character, &after, sizeof(character)));
                }
            }
        if (f->p->substep == 1 && f->p->step == 15)
            ++recovery;
    }
    CHECK(image_ticks && strikes && attacking && recovery == 16 &&
          f->p->step == SOTN_PLAYER_STAND && f->p->palette == 0x8100 && f->spent == 5);
    for (Uint16 i = 0; i < 48; ++i)
        CHECK(f->runtime.children[i].kind != 0xe1 && f->runtime.children[i].kind != 0xf1 &&
              f->runtime.children[i].kind != 0xe2 && f->runtime.children[i].kind != 0xf2);
    CHECK(Tick(f, SOTN_BUTTON_JUMP) && f->p->step == SOTN_PLAYER_JUMP);
    return true;
}
static bool Failure(Fixture *f)
{
    CHECK(Create(f, false));
    CHECK(Tick(f, 0) && Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT));
    Put32(f->dra + 0x101b8 + 0xd9 * 4U, 0x800b1000);
    const SotnPlayer before = *f->p;
    CHECK(SotnPlayer_SetSwordContext(f->p, Factory, f, false));
    const SotnPlayer ready = *f->p;
    CHECK(!SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT_HAND) &&
          !SDL_memcmp(&ready, f->p, sizeof(ready)) && before.mp == 20);
    // The native transaction owner must discard actual reservations after this late failure.
    CHECK(f->runtime.factories[0].cursor.entity_id && f->runtime.factories[1].cursor.entity_id);
    SDL_zero(f->runtime);
    Put32(f->dra + 0x101b8 + 0xd9 * 4U, 0x800a7100);
    for (Uint16 i = 0; i < 8; ++i)
    {
        bool allocated = false;
        CHECK(Factory(f, (SotnPlayerFactorySource){.priority = 148}, 0x14003d, 0, &allocated) &&
              allocated);
    }
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT_HAND) && f->p->step != 15 &&
          f->p->mp == 20 && !f->p->mp_spent);
    return true;
}
static bool Resources(Fixture *f)
{
    for (Uint16 mp = 4; mp <= 5; ++mp)
    {
        CHECK(Create(f, false));
        SotnCharacterState progress = {0};
        CHECK(SotnCharacter_Get(f->character, &progress, NULL));
        progress.mp = mp;
        CHECK(SotnCharacter_Restore(f->character, &progress));
        CHECK(Tick(f, 0) && Tick(f, SOTN_BUTTON_DOWN) &&
              Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
              Tick(f, SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT_HAND));
        CHECK((f->p->step == 15) == (mp == 5) && f->spent == (mp == 5 ? 5 : 0) &&
              f->p->mp == (mp == 5 ? 0 : 4));
    }
    CHECK(Create(f, false) && Tick(f, 0));
    f->p->curse_ticks = 100;
    CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
          Tick(f, SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT_HAND));
    CHECK(f->p->curse_failure && f->p->step != 15 && !f->spent && f->p->mp == 20);
    return true;
}
static bool OptionalExhaustion(Fixture *f)
{
    CHECK(Create(f, false));
    SDL_memset(&f->slots, 0xff, sizeof(f->slots));
    CHECK(Tick(f, 0) && Tick(f, SOTN_BUTTON_DOWN) &&
          Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
          Tick(f, SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT_HAND));
    Uint16 wait = 0;
    while (f->p->step == 15 && wait++ < 100)
    {
        CHECK(Tick(f, 0));
        for (Uint16 i = 0; i < 48; ++i)
            CHECK(f->runtime.children[i].kind != 31);
    }
    CHECK(wait < 100 && f->spent == 5 && f->p->step == SOTN_PLAYER_STAND);
    CHECK(Create(f, false));
    CHECK(Tick(f, 0) && Tick(f, SOTN_BUTTON_DOWN) &&
          Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
          Tick(f, SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT_HAND));
    f->dra[0xd1d4 + 61 * 6 + 5] = 255;
    for (Uint16 i = 0; i < 8; ++i)
    {
        bool allocated = false;
        CHECK(Factory(f, (SotnPlayerFactorySource){.priority = 148}, 0x14003d, 0, &allocated) &&
              allocated);
    }
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &before, NULL));
    f->p->animation.pose = 5;
    f->p->animation.remaining = 2;
    CHECK(Tick(f, 0) && f->runtime.children[0].state.sword.image.phase == 2 &&
          !SotnBatRuntime_Available(&f->runtime));
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == before.random &&
          !f->runtime.sword_mask);
    for (Uint16 i = 1; i < 16; ++i)
        CHECK(f->runtime.children[i].kind != 0xe2);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = CommandAndMotion() && Casting(f, false) && Casting(f, true) && Failure(f) &&
                    Resources(f) && OptionalExhaustion(f);
    SotnPlayer_Close(f->p);
    SotnCharacter_Destroy(f->character);
    SDL_free(f);
    return ok ? 0 : 1;
}
