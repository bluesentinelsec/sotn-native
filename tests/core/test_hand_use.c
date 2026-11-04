// Ordinary hand-use selection from dra/6E42C.c with authored tables only: Red Rust (kind13)
// rolls libc rand()&7 after its actual factory; a completed back-forward command selects the
// equipped unk17 item (Basilard's 174) without the casting grunt; kind0 airborne variants.
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
static void Item(Uint8 *dra, Uint16 id, Uint8 category, Uint8 family, Uint8 kind, Uint8 animation,
                 Uint8 variant, Uint8 lock, Uint8 back_forward)
{
    Uint8 *e = dra + 0x500 + id * 52U;
    Put16(e + 8, 8);
    e[14] = category;
    e[15] = family;
    e[17] = kind;
    e[18] = animation;
    e[19] = 48;
    e[20] = variant;
    e[21] = lock;
    e[22] = 128;
    e[23] = back_forward;
    Put16(e + 40, 2);
}
static bool Create(Fixture *f, Uint16 item, Uint16 first, Uint16 second, bool allocate)
{
    SotnPlayer_Close(f->p);
    SotnCharacter_Destroy(f->character);
    SDL_memset(f, 0, sizeof(*f));
    TestCharacter_Fill(f->dra);
    f->dra[0x500 + 18] = 0x24; // Empty hand item0 animation.
    Item(f->dra, 26, 5, 35, 13, 0x99, 0, 13, 0);
    Item(f->dra, 18, 1, 31, 7, 0x6f, 0, 5, 174);
    Item(f->dra, 174, 1, 31, 8, 0x76, 1, 8, 0);
    Item(f->dra, 85, 3, 1, 0, 0x24, 0, 5, 0);
    Item(f->dra, 130, 4, 4, 3, 0x4f, 0, 11, 170);
    Item(f->dra, 170, 4, 4, 4, 0x56, 1, 13, 0);
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
    CHECK(SotnCharacter_Collect(f->character, false, item) &&
          SotnCharacter_Equip(f->character, SOTN_EQUIP_LEFT, item));
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, f->map, 256, 135, &f->p));
    const SotnDataView weapons = {f->weapons, sizeof(f->weapons)};
    const Uint16 right = item == 26 ? 26 : 0;
    // Two-hand Red Rust and fists require actual shared hand ownership before they bind.
    CHECK(item == 18 || !SotnPlayer_SetEquipment(f->p, weapons, item, right));
    CHECK(SotnPlayer_SetHandContext(f->p, Factory, Random, f));
    CHECK(item != 26 || !SotnPlayer_SetEquipment(f->p, weapons, 26, 0));
    CHECK(SotnPlayer_SetEquipment(f->p, weapons, item, right));
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
    CHECK(Create(f, 26, first, 5, true));
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
static bool BackForward(Fixture *f)
{
    for (Uint16 special = 0; special < 2; ++special)
    {
        CHECK(Create(f, 18, 1, 2, true));
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        if (special)
            CHECK(Tick(f, SOTN_BUTTON_LEFT) && f->p->back_forward.phase == 1 &&
                  Tick(f, SOTN_BUTTON_RIGHT) && f->p->back_forward.phase == 255 && Tick(f, 0));
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
        SotnPlayerState p = {0};
        CHECK(SotnPlayer_GetState(f->p, &p));
        // The virtual item uses its own blueprint parameter and skips the grunt roll.
        CHECK(f->factories == 1 && f->item == (special ? 174 : 18) &&
              f->packed == (0x1030U | (special ? 0x10000U : 0U)) &&
              f->roll_count == (special ? 0 : 1) && p.hand_used == !special &&
              f->p->attack_mode == (special ? 0x8009 : 0x8008) &&
              f->p->substep == (special ? 0x48 : 0x47) && f->p->attack_lock == (special ? 8 : 5) &&
              f->p->animation.id >= (special ? 0x76 : 0x6f) &&
              f->p->animation.id < (special ? 0x76 : 0x6f) + 7);
    }
    return true;
}
// Holy rod's special170 is kind4: it still takes block_1000's casting grunt roll.
static bool KindFourSpecial(Fixture *f)
{
    CHECK(Create(f, 130, 1, 2, true));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Tick(f, 0));
    CHECK(Tick(f, SOTN_BUTTON_LEFT) && Tick(f, SOTN_BUTTON_RIGHT) && Tick(f, 0) &&
          Tick(f, SOTN_BUTTON_LEFT_HAND));
    SotnPlayerState p = {0};
    CHECK(SotnPlayer_GetState(f->p, &p) && f->item == 170 && f->packed == 0x11030U &&
          f->roll_count == 1 && p.hand_used && f->p->attack_mode == 0x8005 &&
          f->p->substep == 0x44 && f->p->attack_lock == 13);
    return true;
}
static bool AirborneKindZero(Fixture *f)
{
    // block_98: kind0 needs Down plus Left/Right for variant6; Down alone stays variant5.
    for (Uint16 sideways = 0; sideways < 2; ++sideways)
    {
        CHECK(Create(f, 85, 1, 2, true));
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        CHECK(Tick(f, SOTN_BUTTON_JUMP));
        for (Uint16 i = 0; i < 60 && f->p->vy <= 0; ++i)
            CHECK(Tick(f, 0));
        CHECK(f->p->vy > 0 && f->p->step == SOTN_PLAYER_JUMP);
        CHECK(Tick(f, (Uint16)(SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_DOWN |
                               (sideways ? SOTN_BUTTON_RIGHT : 0))));
        CHECK(f->factories == 1 && f->p->animation.id == 0x24 + (sideways ? 6 : 5));
    }
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    bool ok = f != NULL;
    // rand()&7: 8 and 0x7ff8 fail; 9 and 7 continue to the ordinary attack.
    ok = ok && Roll(f, 8, true) && Roll(f, 0x7ff8, true) && Roll(f, 9, false) &&
         Roll(f, 7, false) && BackForward(f) && KindFourSpecial(f) && AirborneKindZero(f);
    // Exhausted factories never reach the roll.
    if (ok)
    {
        ok = Create(f, 26, 8, 5, false);
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
