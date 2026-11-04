// Red Rust (kind13) hand use: dra/6E42C.c rolls libc rand()&7 after the actual hand factory;
// zero takes the shared curse failure branch without the casting grunt. Authored tables only.
#include "character_fixture.h"
#include "player_internal.h"

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
    SotnCollisionMap map;
    Uint16 rolls[4], roll_count, factories, item;
    Uint32 packed;
    bool allocate;
} Fixture;
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static bool Factory(void *context, SotnPlayerFactorySource source, Uint32 packed, Uint16 argument,
                    bool *allocated)
{
    (void)source;
    Fixture *f = context;
    ++f->factories;
    f->packed = packed;
    f->item = argument;
    *allocated = f->allocate;
    return true;
}
static bool Random(void *context, Uint16 *value)
{
    Fixture *f = context;
    if (f->roll_count >= SDL_arraysize(f->rolls))
        return SDL_SetError("Red Rust test exhausted its scripted rand results");
    *value = f->rolls[f->roll_count++];
    return true;
}
static bool Create(Fixture *f, Uint16 first, Uint16 second, bool allocate)
{
    SotnPlayer_Close(f->p);
    SotnCharacter_Destroy(f->character);
    SDL_memset(f, 0, sizeof(*f));
    TestCharacter_Fill(f->dra);
    Uint8 *e = f->dra + 0x500 + 26 * 52;
    Put16(e + 8, 8);
    e[14] = 5;
    e[15] = 35;
    e[17] = 13;
    e[18] = 0x99;
    e[19] = 48;
    e[21] = 13;
    e[22] = 128;
    Put16(e + 40, 2);
    for (Uint16 i = 0; i < 247; ++i)
        TestCharacter_Put32(f->dra + 0x101b8 + i * 4U, 0x800a7000);
    for (Uint16 i = 0; i < 20; ++i)
        TestCharacter_Put32(f->dra + 0x7000 + i * 4U, 0x00010002);
    TestCharacter_Put32(f->dra + 0x7050, 0xffff);
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 64; ++x)
            f->tiles[(y * 64U + x) * 2] = 1;
    f->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 64, 16};
    f->rolls[0] = first;
    f->rolls[1] = second;
    f->allocate = allocate;
    CHECK(SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &f->character));
    CHECK(SotnCharacter_Collect(f->character, false, 26) &&
          SotnCharacter_Equip(f->character, SOTN_EQUIP_LEFT, 26));
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, f->map, 256, 135, &f->p));
    // Two-hand Red Rust requires actual shared hand ownership before it can bind.
    CHECK(!SotnPlayer_SetEquipment(f->p, (SotnDataView){f->weapons, sizeof(f->weapons)}, 26, 26));
    CHECK(SotnPlayer_SetHandContext(f->p, Factory, Random, f));
    CHECK(!SotnPlayer_SetEquipment(f->p, (SotnDataView){f->weapons, sizeof(f->weapons)}, 26, 0));
    CHECK(SotnPlayer_SetEquipment(f->p, (SotnDataView){f->weapons, sizeof(f->weapons)}, 26, 26));
    return true;
}
static bool Tick(Fixture *f, Uint16 buttons)
{
    SotnCharacterState progress = {0};
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Get(f->character, &progress, &stats) &&
          SotnPlayer_SetCharacter(f->p, &progress, stats));
    return SotnPlayer_Tick(f->p, buttons);
}
static bool Roll(Fixture *f, Uint16 first, bool fails)
{
    CHECK(Create(f, first, 5, true));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Tick(f, 0));
    CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
    SotnPlayerState p = {0};
    CHECK(SotnPlayer_GetState(f->p, &p));
    // The actual factory always precedes the roll; failure omits the casting grunt.
    CHECK(f->factories == 1 && f->item == 26 && (f->packed & 0xfffU) == 48 &&
          f->roll_count == (fails ? 1 : 2) && f->p->curse_failure == fails &&
          p.hand_used == !fails);
    if (fails)
        CHECK(f->p->attack_mode == 0x8012 && f->p->attack_lock == 255 && f->p->substep == 0x51 &&
              f->p->animation.id == 0xb5 && p.sound == 0x6e7 && !p.hand_grunt);
    else
        CHECK(f->p->animation.id >= 0x99 && f->p->animation.id < 0x99 + 7);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    bool ok = f != NULL;
    // rand()&7: 8 and 0x7ff8 fail; 9 and 7 continue to the ordinary attack.
    ok = ok && Roll(f, 8, true) && Roll(f, 0x7ff8, true) && Roll(f, 9, false) && Roll(f, 7, false);
    // Exhausted factories never reach the roll.
    if (ok)
    {
        ok = Create(f, 8, 5, false);
        for (Uint16 i = 0; ok && i < 8; ++i)
            ok = Tick(f, 0);
        ok = ok && Tick(f, SOTN_BUTTON_LEFT_HAND) && f->factories == 1 && !f->roll_count &&
             !f->p->curse_failure;
        if (!ok)
            SDL_Log("Red Rust exhaustion check failed: %s", SDL_GetError());
    }
    if (f)
    {
        SotnPlayer_Close(f->p);
        SotnCharacter_Destroy(f->character);
    }
    SDL_free(f);
    return ok ? 0 : 1;
}
