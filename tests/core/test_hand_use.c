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
    Uint16 rolls[4], roll_count, factories, item, swords, consumed;
    Uint32 packed, sword_packed;
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
static bool Sword(void *context, SotnPlayerFactorySource source, Uint32 packed, Sint16 argument,
                  bool *allocated)
{
    (void)source;
    (void)argument;
    Fixture *f = context;
    ++f->swords;
    f->sword_packed = packed;
    *allocated = true;
    return true;
}
static bool Consume(void *context, Uint16 hand, Uint16 item, Uint16 *equipped)
{
    (void)hand;
    (void)item;
    (void)equipped;
    ++((Fixture *)context)->consumed;
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
    Item(f->dra, 20, 0, 32, 9, 0x7d, 0, 5, 175);
    f->dra[0x500 + 20 * 52 + 24] = 175;
    Item(f->dra, 175, 0, 32, 10, 0x84, 1, 8, 0);
    Item(f->dra, 121, 1, 54, 1, 0x41, 0, 13, 0);
    f->dra[0x500 + 121 * 52 + 24] = 195; // QCF specialMove
    Item(f->dra, 195, 1, 54, 1, 0x41, 1, 13, 0);
    Item(f->dra, 115, 1, 55, 1, 0x41, 0, 13, 0);
    f->dra[0x500 + 115 * 52 + 24] = 196; // QCF specialMove
    Item(f->dra, 196, 1, 55, 28, 0x41, 1, 13, 0);
    Put16(f->dra + 0x500 + 196 * 52 + 36, 5);
    Item(f->dra, 23, 0, 38, 16, 0xae, 0, 13, 0);
    f->dra[0x500 + 23 * 52 + 24] = 191; // QCF specialMove
    Item(f->dra, 191, 0, 38, 26, 0xae, 1, 13, 0);
    Item(f->dra, 112, 1, 48, 1, 0x41, 0, 13, 0);
    f->dra[0x500 + 112 * 52 + 24] = 178; // QCF specialMove
    Item(f->dra, 178, 1, 48, 19, 0x41, 1, 13, 0);
    Put16(f->dra + 0x500 + 178 * 52 + 36, 5);
    Item(f->dra, 139, 5, 53, 1, 0x41, 2, 13, 192);
    f->dra[0x500 + 139 * 52 + 24] = 193;
    Item(f->dra, 192, 5, 53, 1, 0x41, 3, 13, 0);
    Item(f->dra, 193, 5, 53, 135, 0xd8, 0, 0, 0);
    f->dra[0x500 + 193 * 52 + 19] = 60;
    Item(f->dra, 102, 3, 1, 0, 0x24, 2, 5, 0);
    f->dra[0x500 + 102 * 52 + 24] = 186;
    Item(f->dra, 186, 3, 1, 24, 0x24, 0, 5, 0);
    f->dra[0x500 + 186 * 52 + 19] = 54;
    Item(f->dra, 120, 3, 1, 0, 0x24, 3, 5, 187);
    f->dra[0x500 + 120 * 52 + 24] = 188;
    Item(f->dra, 187, 3, 1, 24, 0x24, 1, 5, 0);
    f->dra[0x500 + 187 * 52 + 19] = 54;
    Item(f->dra, 188, 2, 1, 25, 0x24, 4, 7, 0);
    Item(f->dra, 111, 1, 49, 1, 0x41, 0, 13, 0);
    f->dra[0x500 + 111 * 52 + 24] = 179;
    Item(f->dra, 179, 1, 49, 20, 0x41, 1, 13, 0);
    Put16(f->dra + 0x500 + 179 * 52 + 36, 5);
    Item(f->dra, 113, 1, 50, 1, 0x41, 0, 13, 0);
    f->dra[0x500 + 113 * 52 + 24] = 181;
    Item(f->dra, 181, 1, 50, 22, 0x41, 1, 13, 0);
    Put16(f->dra + 0x500 + 181 * 52 + 36, 5);
    Item(f->dra, 132, 4, 6, 2, 0x48, 0, 11, 0);
    f->dra[0x500 + 132 * 52 + 24] = 172;
    Item(f->dra, 172, 4, 6, 5, 0x66, 0, 0, 0);
    f->dra[0x500 + 172 * 52 + 19] = 72;
    f->dra[0x500 + 172 * 52 + 22] = 1;
    Put16(f->dra + 0x500 + 172 * 52 + 36, 5);
    Item(f->dra, 98, 5, 51, 6, 0x68, 0, 15, 182);
    Item(f->dra, 182, 1, 51, 23, 0xd6, 0, 13, 0);
    f->dra[0x500 + 182 * 52 + 19] = 54;
    Put16(f->dra + 0x500 + 182 * 52 + 36, 5);
    Item(f->dra, 95, 5, 51, 27, 0xd6, 8, 18, 0);
    Item(f->dra, 141, 5, 56, 29, 0x41, 0, 13, 0);
    Item(f->dra, 165, 5, 53, 129, 0x5d, 0, 0, 0);
    f->dra[0x500 + 165 * 52 + 19] = 68;
    f->dra[0x500 + 165 * 52 + 22] = 1;
    Item(f->dra, 133, 2, 12, 130, 0x60, 0, 0, 0);
    f->dra[0x500 + 133 * 52 + 19] = 56;
    f->dra[0x500 + 133 * 52 + 22] = 2;
    Item(f->dra, 163, 1, 12, 129, 0x5d, 1, 0, 0);
    f->dra[0x500 + 163 * 52 + 19] = 50;
    f->dra[0x500 + 163 * 52 + 22] = 3;
    Item(f->dra, 164, 1, 12, 129, 0x5d, 2, 0, 0);
    f->dra[0x500 + 164 * 52 + 19] = 52;
    f->dra[0x500 + 164 * 52 + 22] = 3;
    Item(f->dra, 143, 2, 14, 129, 0x5d, 0, 0, 0);
    Item(f->dra, 119, 2, 13, 129, 0x5d, 0, 0, 0);
    f->dra[0x500 + 119 * 52 + 19] = 50;
    f->dra[0x500 + 119 * 52 + 22] = 1;
    f->dra[0x500 + 143 * 52 + 19] = 50;
    f->dra[0x500 + 143 * 52 + 22] = 1;
    Item(f->dra, 130, 4, 4, 3, 0x4f, 0, 11, 170);
    // Mablung124 (combo main0x3FFE) with Leather shield5 (sub2) casts spell203 (sub2).
    Item(f->dra, 124, 1, 57, 1, 0x41, 3, 13, 0);
    TestCharacter_Put32(f->dra + 0x500 + 124 * 52 + 0x20, 0x3ffe);
    Item(f->dra, 5, 9, 8, 128, 0x63, 0, 0, 0);
    TestCharacter_Put32(f->dra + 0x500 + 5 * 52 + 0x1c, 2);
    Item(f->dra, 203, 10, 8, 5, 0xba, 0, 0, 0);
    f->dra[0x500 + 203 * 52 + 19] = 86;
    f->dra[0x500 + 203 * 52 + 22] = 1;
    Put16(f->dra + 0x500 + 203 * 52 + 36, 10);
    TestCharacter_Put32(f->dra + 0x500 + 203 * 52 + 0x1c, 2);
    // Axe Lord shield8 (sub0x10) casts spell206 (category1) for twenty MP.
    Item(f->dra, 8, 9, 11, 128, 0x63, 0, 0, 0);
    TestCharacter_Put32(f->dra + 0x500 + 8 * 52 + 0x1c, 0x10);
    Item(f->dra, 206, 1, 11, 5, 0xba, 0, 0, 0);
    f->dra[0x500 + 206 * 52 + 19] = 86;
    f->dra[0x500 + 206 * 52 + 22] = 1;
    Put16(f->dra + 0x500 + 206 * 52 + 36, 20);
    TestCharacter_Put32(f->dra + 0x500 + 206 * 52 + 0x1c, 0x10);
    // Goddess shield11 (sub0x80) casts spell209 for forty MP.
    Item(f->dra, 11, 9, 25, 128, 0x63, 0, 0, 0);
    TestCharacter_Put32(f->dra + 0x500 + 11 * 52 + 0x1c, 0x80);
    Item(f->dra, 209, 10, 25, 5, 0xba, 0, 0, 0);
    f->dra[0x500 + 209 * 52 + 19] = 86;
    f->dra[0x500 + 209 * 52 + 22] = 1;
    Put16(f->dra + 0x500 + 209 * 52 + 36, 40);
    TestCharacter_Put32(f->dra + 0x500 + 209 * 52 + 0x1c, 0x80);
    Item(f->dra, 17, 5, 30, 6, 0x68, 0, 13, 0);
    f->dra[0x500 + 17 * 52 + 24] = 177; // QCF specialMove
    Item(f->dra, 177, 1, 30, 5, 0xb8, 0, 0, 0);
    f->dra[0x500 + 177 * 52 + 19] = 56;
    f->dra[0x500 + 177 * 52 + 22] = 8;
    Put16(f->dra + 0x500 + 177 * 52 + 36, 5);
    Item(f->dra, 28, 0, 34, 12, 0x92, 0, 13, 0);
    f->dra[0x500 + 28 * 52 + 24] = 176; // QCF specialMove
    Item(f->dra, 176, 1, 34, 129, 0x5d, 0, 0, 0);
    f->dra[0x500 + 176 * 52 + 19] = 56;
    f->dra[0x500 + 176 * 52 + 22] = 1;
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
    const Uint16 right = item == 26 || item == 139 || item == 98 || item == 95 || item == 141 ||
                                 item == 165 || item == 17
                             ? item
                             : 0;
    // Two-hand Red Rust and fists require actual shared hand ownership before they bind.
    // Runesword143 binds through its kind129 throwing owner, not the ordinary hand context.
    CHECK(item == 18 || item == 143 || item == 119 || item == 133 || item == 163 || item == 164 ||
          item == 165 || !SotnPlayer_SetEquipment(f->p, weapons, item, right));
    CHECK(SotnPlayer_SetHandContext(f->p, Factory, Random, f) &&
          SotnPlayer_SetSwordContext(f->p, Sword, f, false));
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
// 71830.c substep0x40: kind0 fists continue at pose7 while the attack button is held.
static bool FistCombo(Fixture *f)
{
    CHECK(Create(f, 85, 1, 2, true));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Tick(f, 0));
    CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND) && f->p->substep == 0x40);
    for (Uint16 i = 0; i < 80 && f->p->animation.pose < 7; ++i)
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
    CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
    CHECK(f->p->animation.pose == 7);
    for (Uint16 i = 0; i < 6; ++i)
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND) && f->p->animation.pose == 7 &&
              f->p->animation.remaining == 1 && f->p->substep == 0x40);
    for (Uint16 i = 0; i < 6 && f->p->substep == 0x40; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->p->substep != 0x40 && !f->p->attack_mode);
    return true;
}
// 6E42C.c block_32: a grounded completed QCF selects Gurthang's specialMove195 (kind1, grunt);
// airborne or incomplete commands keep the ordinary item.
static bool QuarterCircle(Fixture *f)
{
    for (Uint16 grounded = 0; grounded < 2; ++grounded)
    {
        CHECK(Create(f, 121, 1, 2, true));
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        if (!grounded)
        {
            CHECK(Tick(f, SOTN_BUTTON_JUMP));
            for (Uint16 i = 0; i < 4; ++i)
                CHECK(Tick(f, 0));
        }
        CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
              Tick(f, SOTN_BUTTON_RIGHT) && f->p->sword_command.phase == 255);
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
        SotnPlayerState p = {0};
        CHECK(SotnPlayer_GetState(f->p, &p) && f->factories == 1 &&
              f->item == (grounded ? 195 : 121) && f->packed == (grounded ? 0x11030U : 0x1030U) &&
              f->roll_count == 1 && p.hand_used);
    }
    // The weapon54 timer request only raises Dark Metamorphosis to its bounded floor.
    CHECK(SotnPlayer_RaiseDarkTimer(f->p, 0x38) && f->p->dark_ticks == 0x38);
    f->p->dark_ticks = 0x100;
    CHECK(SotnPlayer_RaiseDarkTimer(f->p, 0x38) && f->p->dark_ticks == 0x100 &&
          !SotnPlayer_RaiseDarkTimer(f->p, 0) && !SotnPlayer_RaiseDarkTimer(f->p, 0x39));
    return true;
}
// 6E42C.c kind28 (Holy sword special196): grounded QCF, five MP after the factory, no casting
// grunt, standing still with SFX_VO_ALU_ATTACK_B; 71830.c substep0x5C holds pose0 ten updates.
static bool HolySword(Fixture *f)
{
    CHECK(Create(f, 115, 1, 2, true));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Tick(f, 0));
    const Uint16 mp = f->p->mp;
    f->p->vx = 0x18000;
    CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
          Tick(f, SOTN_BUTTON_RIGHT) && f->p->sword_command.phase == 255);
    CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT));
    SotnPlayerState p = {0};
    CHECK(SotnPlayer_GetState(f->p, &p) && f->factories == 1 && f->item == 196 &&
          f->packed == 0x11030U && !f->roll_count && !p.hand_used && p.sound == 0x6ef &&
          f->p->mp == mp - 5 && f->p->mp_spent == 5 && f->p->step == SOTN_PLAYER_STAND &&
          !f->p->vx && f->p->substep == 0x5c && f->p->attack_mode == 0x801d &&
          f->p->attack_hold == 10 && f->p->animation.pose == 0);
    for (Sint32 t = 1; t <= 10; ++t)
    {
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND) && f->p->attack_hold == 10 - t &&
              f->p->substep == 0x5c && f->p->step == SOTN_PLAYER_STAND);
        CHECK(f->p->animation.pose == (t < 10 ? 0 : 1));
    }
    // Without five MP the QCF falls back to the ordinary Holy sword attack.
    CHECK(Create(f, 115, 1, 2, true));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Tick(f, 0));
    SotnCharacterState before = {0};
    CHECK(SotnCharacter_Get(f->character, &before, NULL) &&
          SotnCharacter_SpendMP(f->character, (Uint16)(before.mp - 4U)));
    CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
          Tick(f, SOTN_BUTTON_RIGHT) && Tick(f, SOTN_BUTTON_LEFT_HAND));
    CHECK(f->item == 115 && f->roll_count == 1 && f->p->mp == 4 && f->p->substep == 0x41);
    return true;
}
// 6E42C.c kind19 (Thunderbrand special178): like kind28 but holding pose0 for forty updates in
// substep0x53, sharing 71830.c's 0x53..0x56/0x5C handler.
static bool Brand(Fixture *f)
{
    CHECK(Create(f, 112, 1, 2, true));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Tick(f, 0));
    const Uint16 mp = f->p->mp;
    CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
          Tick(f, SOTN_BUTTON_RIGHT) && Tick(f, SOTN_BUTTON_LEFT_HAND));
    SotnPlayerState p = {0};
    CHECK(SotnPlayer_GetState(f->p, &p) && f->item == 178 && f->packed == 0x11030U &&
          !f->roll_count && p.sound == 0x6ef && f->p->mp == mp - 5 && f->p->substep == 0x53 &&
          f->p->attack_mode == 0x8014 && f->p->attack_hold == 40);
    for (Sint32 t = 1; t <= 40; ++t)
        CHECK(Tick(f, 0) && f->p->attack_hold == 40 - t &&
              f->p->animation.pose == (t < 40 ? 0 : 1));
    // Firebrand111's QCF special179 is kind20 (substep0x54) on the same hold.
    CHECK(Create(f, 111, 1, 2, true));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Tick(f, 0));
    const Uint16 fire_mp = f->p->mp;
    CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
          Tick(f, SOTN_BUTTON_RIGHT) && Tick(f, SOTN_BUTTON_LEFT_HAND));
    CHECK(f->item == 179 && f->packed == 0x11030U && !f->roll_count && f->p->mp == fire_mp - 5 &&
          f->p->substep == 0x54 && f->p->attack_mode == 0x8015 && f->p->attack_hold == 40);
    // Icebrand113's QCF special181 is kind22 (substep0x56) on the same hold.
    CHECK(Create(f, 113, 1, 2, true));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Tick(f, 0));
    const Uint16 ice_mp = f->p->mp;
    CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
          Tick(f, SOTN_BUTTON_RIGHT) && Tick(f, SOTN_BUTTON_LEFT_HAND));
    CHECK(f->item == 181 && f->packed == 0x11030U && !f->roll_count && f->p->mp == ice_mp - 5 &&
          f->p->substep == 0x56 && f->p->attack_mode == 0x8017 && f->p->attack_hold == 40);
    return true;
}
// 6E42C.c case135 (Osafune special193): blueprint60 reservation, VO_ALU_ATTACK_C, no grunt and
// func_8010ED54's sword-warp step, animation0xD8 and profile20 outline; back-forward keeps192.
static bool Warp(Fixture *f)
{
    CHECK(Create(f, 139, 1, 2, true));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Tick(f, 0));
    f->p->vx = 0x18000;
    CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
          Tick(f, SOTN_BUTTON_RIGHT) && Tick(f, SOTN_BUTTON_LEFT_HAND));
    SotnPlayerState p = {0};
    CHECK(SotnPlayer_GetState(f->p, &p) && f->factories == 1 && f->item == 193 &&
          f->packed == 0x103cU && !f->roll_count && p.sound == 0x6f0 &&
          f->p->step == SOTN_PLAYER_SWORD_WARP && f->p->animation.id == 0xd8 && !f->p->vx &&
          !f->p->vy && f->swords == 1 && f->sword_packed == 0x14003dU);
    CHECK(Create(f, 139, 1, 2, true));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Tick(f, 0));
    CHECK(Tick(f, SOTN_BUTTON_LEFT) && Tick(f, SOTN_BUTTON_RIGHT) && Tick(f, 0) &&
          Tick(f, SOTN_BUTTON_LEFT_HAND));
    CHECK(f->item == 192 && f->packed == 0x31030U && f->p->step == SOTN_PLAYER_STAND &&
          f->roll_count == 1);
    return true;
}
// 6E42C.c kind26 (Rapier special191): grounded QCF, no grunt, stops both velocities and plays
// SFX_VO_ALU_ATTACK_B; 71830.c substep0x5A holds pose6 while forty updates last, and a Down tap
// or the backward direction resumes the ordinary substep0x50.
static bool Lunge(Fixture *f)
{
    for (Uint16 k = 0; k < 3; ++k)
    {
        CHECK(Create(f, 23, 1, 2, true));
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
              Tick(f, SOTN_BUTTON_RIGHT) && f->p->sword_command.phase == 255);
        f->p->vx = 0x18000;
        f->p->vy = 0x1000;
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
        SotnPlayerState p = {0};
        CHECK(SotnPlayer_GetState(f->p, &p) && f->factories == 1 && f->item == 191 &&
              f->packed == 0x11030U && !f->roll_count && p.sound == 0x6ef &&
              f->p->substep == 0x5a && f->p->attack_mode == 0x801b && f->p->attack_hold == 40 &&
              !f->p->vx && !f->p->vy && f->p->step == SOTN_PLAYER_STAND);
        if (k == 0)
        {
            Uint16 held = 0, t = 0;
            for (; t < 80 && f->p->substep == 0x5a; ++t)
            {
                CHECK(Tick(f, 0));
                held += f->p->animation.pose == 6;
            }
            // Pose6 holds for the rest of the forty-update countdown (authored two-update poses
            // reach it after twelve), then pose10 resumes substep0x50.
            CHECK(held == 28 && f->p->substep == 0x50 && !f->p->attack_hold &&
                  f->p->animation.pose == 10);
        }
        else
        {
            CHECK(Tick(f, 0) && Tick(f, k == 1 ? SOTN_BUTTON_DOWN : SOTN_BUTTON_LEFT) &&
                  f->p->substep == 0x50 && f->p->attack_hold == 38);
        }
    }
    return true;
}
// 6E42C.c kind24 (Iron Fist special186/Tulkas187): blueprint54, no grunt, both velocities
// stopped, SFX_VO_ALU_ATTACK_B and a0x80 hold; an airborne player stays in Player_Jump, which
// hovers in substep0x58. Tulkas's QCF188 is kind25 (block_98 directly, unarmed substep0x59).
static bool Punches(Fixture *f)
{
    for (Uint16 k = 0; k < 4; ++k)
    {
        // QCF is grounded; the airborne case uses Tulkas's back-forward rush187.
        const bool airborne = k == 3;
        CHECK(Create(f, airborne ? 120 : 102, 1, 2, true));
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        if (airborne)
        {
            for (Uint16 i = 0; i < 5; ++i)
                CHECK(Tick(f, i ? 0 : SOTN_BUTTON_JUMP));
            CHECK(Tick(f, SOTN_BUTTON_LEFT) && Tick(f, SOTN_BUTTON_RIGHT) && Tick(f, 0) &&
                  f->p->back_forward.phase == 255 && f->p->step == SOTN_PLAYER_JUMP);
        }
        else
            CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
                  Tick(f, SOTN_BUTTON_RIGHT) && f->p->sword_command.phase == 255);
        f->p->vx = 0x18000;
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
        SotnPlayerState p = {0};
        CHECK(SotnPlayer_GetState(f->p, &p) && f->factories == 1 &&
              f->item == (airborne ? 187 : 186) && f->packed == (airborne ? 0x11036U : 0x1036U) &&
              !f->roll_count && p.sound == 0x6ef && f->p->substep == 0x58 &&
              f->p->attack_mode == 0x8019 && f->p->attack_hold == 0x80 && !f->p->vx && !f->p->vy &&
              f->p->step == (airborne ? SOTN_PLAYER_JUMP : SOTN_PLAYER_STAND) &&
              f->p->animation.id == (airborne ? 0x28 : 0x24));
        if (k == 0)
        {
            Uint16 t = 0, rewound = 0, last = 0;
            for (; t < 200 && f->p->substep == 0x58; ++t)
            {
                CHECK(Tick(f, 0));
                // Pose4's rewind to pose2 advances to pose3 within the same update.
                rewound += last == 4 && f->p->animation.pose == 3;
                last = f->p->animation.pose;
            }
            // 0x80 updates: the rush loops poses3..4 (two updates each) until the hold ends.
            CHECK(t == 133 && rewound == 30 && f->p->substep == 0x40 && !f->p->attack_hold &&
                  f->p->animation.pose == 6);
        }
        else if (k < 3)
            CHECK(Tick(f, 0) && Tick(f, k == 1 ? SOTN_BUTTON_DOWN : SOTN_BUTTON_LEFT) &&
                  f->p->substep == 0x40 && f->p->attack_hold == 0x7e);
        else
        {
            const Sint32 y = f->p->y.value;
            for (Uint16 i = 0; i < 6; ++i)
                CHECK(Tick(f, 0) && f->p->substep == 0x58 && f->p->y.value == y && !f->p->vy &&
                      f->p->step == SOTN_PLAYER_JUMP);
            CHECK(Tick(f, SOTN_BUTTON_UP) && f->p->substep == 0x40);
            CHECK(Tick(f, 0) && f->p->vy > 0);
        }
    }
    CHECK(Create(f, 120, 1, 2, true));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Tick(f, 0));
    CHECK(Tick(f, SOTN_BUTTON_LEFT) && Tick(f, SOTN_BUTTON_RIGHT) && Tick(f, 0) &&
          Tick(f, SOTN_BUTTON_LEFT_HAND));
    CHECK(f->item == 187 && f->packed == 0x11036U && f->p->substep == 0x58 &&
          f->p->attack_hold == 0x80 && !f->roll_count);
    CHECK(Create(f, 120, 1, 2, true));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Tick(f, 0));
    CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
          Tick(f, SOTN_BUTTON_RIGHT) && Tick(f, SOTN_BUTTON_LEFT_HAND));
    SotnPlayerState p = {0};
    CHECK(SotnPlayer_GetState(f->p, &p) && f->item == 188 && f->packed == 0x41030U &&
          !f->roll_count && !p.hand_used && p.sound != 0x6ef && f->p->substep == 0x59 &&
          f->p->attack_mode == 0x801a && f->p->attack_lock == 7 && !f->p->attack_hold);
    return true;
}
// 6E42C.c kind23 (Claymore98's back-forward special182): blueprint54, no grunt, SFX_VO_ALU_
// ATTACK_B, speed5 halved by block_98 and five MP after the factory only when MP covers it;
// 71830.c substep0x57 glides at 1/16 deceleration with grounded dust on odd game-timer updates.
// Kind27 (Estoc95): speed4 halved, its own step (airborne stays Player_Jump) and substep0x5B;
// in a cramped space it keeps the factory but starts no attack. In the air the ledge variant
// (IN_AIR_OR_EDGE) selects animation0xD7.
static bool Weapon51Specials(Fixture *f)
{
    for (Uint16 poor = 0; poor < 2; ++poor)
    {
        CHECK(Create(f, 98, 1, 2, true));
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        if (poor)
        {
            SotnCharacterState before = {0};
            CHECK(SotnCharacter_Get(f->character, &before, NULL) &&
                  SotnCharacter_SpendMP(f->character, (Uint16)(before.mp - 4U)));
        }
        CHECK(Tick(f, SOTN_BUTTON_LEFT) && Tick(f, SOTN_BUTTON_RIGHT) && Tick(f, 0) &&
              f->p->back_forward.phase == 255);
        const Uint16 mp = f->p->mp;
        CHECK(mp == (poor ? 4 : 20));
        f->p->mist_game_timer = 1;
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
        SotnPlayerState p = {0};
        CHECK(SotnPlayer_GetState(f->p, &p) && f->factories == 1 && f->item == 182 &&
              f->packed == 0x1036U && !f->roll_count && !p.hand_used && p.sound == 0x6ef &&
              f->p->mp == (poor ? mp : mp - 5) && f->p->substep == 0x57 &&
              f->p->attack_mode == 0x8018 && f->p->step == SOTN_PLAYER_STAND &&
              f->p->vx == 163840 && !f->p->vy && f->p->animation.id == 0xd6);
        // The lunge decelerates by 1/16 and raises dust on odd timer updates while grounded.
        Uint16 dust = 0;
        for (Uint16 t = 0; t < 6; ++t)
        {
            const Sint32 vx = f->p->vx;
            f->p->mist_game_timer = t;
            CHECK(Tick(f, 0) && f->p->substep == 0x57);
            CHECK(f->p->vx == vx - 4096);
            dust +=
                f->p->movement_effects.smoke && f->p->movement_effects.smoke_blueprint == 0x10045;
            CHECK(f->p->movement_effects.smoke == ((t & 1U) != 0));
        }
        CHECK(dust == 3);
    }
    for (Uint16 k = 0; k < 3; ++k)
    {
        CHECK(Create(f, 95, 1, 2, true));
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        if (k == 1)
        {
            CHECK(Tick(f, SOTN_BUTTON_JUMP));
            for (Uint16 i = 0; i < 3; ++i)
                CHECK(Tick(f, 0));
            CHECK(f->p->step == SOTN_PLAYER_JUMP);
        }
        if (k == 2)
        {
            // A low ceiling over the player makes the actual tight-space probe report cramped.
            for (Uint16 x = 0; x < 64; ++x)
                f->tiles[(7U * 64U + x) * 2] = 1;
            for (Uint16 i = 0; i < 4; ++i)
                CHECK(Tick(f, 0));
        }
        const Sint32 vy = f->p->vy;
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
        SotnPlayerState p = {0};
        CHECK(SotnPlayer_GetState(f->p, &p) && f->factories == 1 && f->item == 95 &&
              f->packed == 0x81030U && !f->roll_count);
        if (k == 2)
            CHECK(!f->p->attack_mode && f->p->substep != 0x5b);
        else
            CHECK(p.sound == 0x6ef && f->p->substep == 0x5b && f->p->attack_mode == 0x801c &&
                  f->p->animation.id == (k ? 0xd7 : 0xd6) && f->p->attack_lock == 18 &&
                  f->p->step == (k ? SOTN_PLAYER_JUMP : SOTN_PLAYER_STAND) &&
                  (k ? f->p->vy != 0 || vy == 0 : !f->p->vy));
    }
    // Animation0x6C's end in Player_Jump resumes the fall and holds back-forward in0xFE.
    CHECK(Create(f, 95, 1, 2, true));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Tick(f, 0));
    f->p->y.parts.high = 60;
    f->p->contact = 0;
    f->p->step = SOTN_PLAYER_JUMP;
    f->p->substep = 0x57;
    f->p->attack_mode = 0x8018;
    f->p->animation.id = 0x6c;
    f->p->animation.remaining = -1;
    f->p->vy = 2 * 65536;
    CHECK(Tick(f, 0) && f->p->substep == 1 && f->p->animation.id == 31 && !f->p->attack_mode &&
          f->p->back_forward.phase == 0xfe && f->p->back_forward.timer <= 0x10 &&
          f->p->back_forward.timer >= 0x0f);
    return true;
}
// 6E42C.c case5 (Moon rod172): blueprint72, the casting grunt and five MP after the factory;
// grounded starts stop with substep0x45, airborne ones keep their state. CheckChainLimit refuses
// the chain1 special while a crescent of that hand lives, falling back to the swing.
static bool MoonRod(Fixture *f)
{
    for (Uint16 k = 0; k < 3; ++k)
    {
        CHECK(Create(f, 132, 1, 2, true));
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        if (k == 1)
        {
            CHECK(Tick(f, SOTN_BUTTON_JUMP));
            for (Uint16 i = 0; i < 2; ++i)
                CHECK(Tick(f, 0));
        }
        if (k == 2)
            CHECK(SotnPlayer_SetSpecialChain(f->p, 1, 0));
        const Uint16 mp = f->p->mp;
        const SotnPlayerStep step = f->p->step;
        CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
              Tick(f, SOTN_BUTTON_RIGHT) && Tick(f, SOTN_BUTTON_LEFT_HAND));
        SotnPlayerState p = {0};
        CHECK(SotnPlayer_GetState(f->p, &p));
        if (k == 2)
            CHECK(f->item == 132 && f->packed == 0x1030U && f->p->mp == mp);
        else if (k == 1)
            CHECK(f->item == 132 || (f->item == 172 && f->p->step == step && !f->p->attack_mode));
        else
            CHECK(f->item == 172 && f->packed == 0x1048U && f->roll_count == 1 && p.hand_used &&
                  f->p->mp == mp - 5 && f->p->substep == 0x45 && f->p->attack_mode == 0x8006 &&
                  f->p->animation.id == 0x66 && !f->p->vx && f->p->step == SOTN_PLAYER_STAND);
    }
    CHECK(!SotnPlayer_SetSpecialChain(f->p, 49, 0));
    return true;
}
// 6E42C.c kind129 Runesword: blueprint50 with the hand's variant, the casting grunt roll, chain1
// against live children carrying143, and ReduceWeapon leaving the non-consumable hand intact.
static bool Runesword(Fixture *f, Uint16 item)
{
    // Weapon12: Chakram133 (blueprint56, kind130 animation0x60) and Vorpal163/Crissaegrim164
    // (blueprints50/52 with descriptors1/2) take the same path.
    const Uint32 packed = item == 133   ? 0x1038U
                          : item == 163 ? 0x1032U | 1U << 16
                          : item == 164 ? 0x1034U | 2U << 16
                          : item == 165 ? 0x1044U
                                        : 0x1032U;
    const Uint8 limit = item == 133 ? 2 : item == 163 || item == 164 ? 3 : 1;
    for (Uint16 k = 0; k < 2; ++k)
    {
        CHECK(Create(f, item, 1, 2, true));
        CHECK(SotnPlayer_SetThrowingContext(f->p, Factory, Consume, Random, f,
                                            (Uint8)(k ? limit : limit - 1U), 0));
        Put16(f->dra + 0xcf62, 0x6f2); // Grunt voice1 of func_8010EA54(8)'s table.
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
        SotnPlayerState p = {0};
        CHECK(SotnPlayer_GetState(f->p, &p));
        if (k)
            CHECK(!f->factories && !f->roll_count && !p.throwing.used);
        else
            CHECK(f->factories == 1 && f->item == item && f->packed == packed &&
                  f->roll_count == 1 && p.throwing.used && p.throwing.keep && !f->consumed &&
                  p.throwing_grunt == 0x6f2 && f->p->animation.id == (item == 133 ? 0x60 : 0x5d) &&
                  f->p->equipment[0] == item);
    }
    return true;
}
// 6E42C.c both buttons: Mablung124 + Leather5 resolve CheckWeaponCombo to spell203 on the
// shield's hand; the spell's factory takes <<14 hand bits, ten MP and kind5's grounded stop.
// A live spell (chain1) or too little MP leaves the left hand's ordinary swing.
static bool Combo(Fixture *f, Uint16 shield, Uint16 spell, Uint16 cost)
{
    for (Uint16 k = 0; k < 4; ++k)
    {
        const Uint16 left = k == 3 ? shield : 124, right = k == 3 ? 124 : shield;
        CHECK(Create(f, left, 1, 2, true));
        CHECK(SotnCharacter_Collect(f->character, false, right) &&
              SotnCharacter_Equip(f->character, SOTN_EQUIP_RIGHT, right) &&
              SotnPlayer_SetEquipment(f->p, (SotnDataView){f->weapons, sizeof(f->weapons)}, left,
                                      right));
        CHECK(SotnPlayer_Combo(f->p) == (k == 3 ? spell : 0x80000000U | spell));
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        CHECK(SotnPlayer_SetComboChain(f->p, k == 1));
        if (f->p->mp < cost)
        {
            // Goddess costs forty: raise the fixture's MP (and maximum) first.
            SotnCharacterState progress = {0};
            CHECK(SotnCharacter_Get(f->character, &progress, NULL));
            progress.mp = progress.mp_max = cost;
            CHECK(SotnCharacter_Restore(f->character, &progress));
            f->p->mp = cost;
        }
        if (k == 2)
        {
            SotnCharacterState progress = {0};
            CHECK(SotnCharacter_Get(f->character, &progress, NULL));
            progress.mp = (Uint16)(cost - 1U);
            CHECK(SotnCharacter_Restore(f->character, &progress));
            f->p->mp = (Uint16)(cost - 1U);
        }
        const Uint16 mp = f->p->mp;
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT_HAND));
        if (k == 0 || k == 3)
            CHECK(f->item == spell && f->packed == (k == 3 ? 0x4056U : 0x808056U) &&
                  f->p->mp == mp - cost && f->p->substep == 0x45 && f->p->attack_mode == 0x8006 &&
                  f->p->animation.id == 0xba);
        else
            CHECK(f->item == 124 && f->packed == 0x31030U && f->p->mp == mp &&
                  f->p->attack_mode == 0x8002);
    }
    CHECK(!SotnPlayer_SetComboChain(f->p, 49));
    // Leather203, Knight204, AxeLord206, Goddess209 and Alucart215 have native spells; Shaman210
    // not yet.
    CHECK(SotnPlayer_ComboSupported(203) && SotnPlayer_ComboSupported(204) &&
          SotnPlayer_ComboSupported(206) && SotnPlayer_ComboSupported(209) &&
          SotnPlayer_ComboSupported(215) && !SotnPlayer_ComboSupported(210));
    return true;
}
// 6E42C.c Sword of Dawn17 (two-handed kind6): its QCF special177 is kind5 (blueprint56, chain8,
// five MP after the factory, grounded substep0x45). The QCF also refuses ground with vram flag
// 0x40, falling back to the ordinary swing.
static bool SwordOfDawn(Fixture *f)
{
    for (Uint16 k = 0; k < 3; ++k)
    {
        CHECK(Create(f, 17, 1, 2, true));
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        CHECK(SotnPlayer_SetSpecialChain(f->p, k == 2 ? 8 : 7, 0));
        const Uint16 mp = f->p->mp;
        CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
              Tick(f, SOTN_BUTTON_RIGHT));
        if (k == 1)
            f->p->contact |= 0x40;
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
        SotnPlayerState p = {0};
        CHECK(SotnPlayer_GetState(f->p, &p) && p.hand_used && f->roll_count == 1);
        if (k == 0)
            CHECK(f->item == 177 && f->packed == 0x1038U && f->p->mp == mp - 5 &&
                  f->p->substep == 0x45 && f->p->attack_mode == 0x8006 &&
                  f->p->animation.id == 0xb8 && f->p->step == SOTN_PLAYER_STAND);
        else
            CHECK(f->item == 17 && f->packed == 0x1030U && f->p->mp == mp &&
                  f->p->attack_mode == 0x8007);
    }
    return true;
}
// 6E42C.c Shotel28 (kind12): its QCF special176 is a kind129 throw (blueprint56, grunt, the
// throw animation, never consumed). While a176 lives with either hand's params, CheckChainLimit
// refuses the special and case12 turns the swing into a punch: Shotel's factory, then the
// empty-hand factory42, with item0's animation, kind and lock.
static bool Shotel(Fixture *f)
{
    for (Uint16 k = 0; k < 4; ++k)
    {
        CHECK(Create(f, 28, 1, 2, true));
        CHECK(SotnPlayer_SetThrowingContext(f->p, Factory, Consume, Random, f, 0, 0));
        Put16(f->dra + 0xcf62, 0x6f2);
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        CHECK(SotnPlayer_SetSpecialChain(f->p, k == 1, 0) &&
              SotnPlayer_SetSpecialCross(f->p, k == 2, 0));
        const Uint16 mp = f->p->mp;
        if (k < 2)
            CHECK(Tick(f, SOTN_BUTTON_DOWN) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT) &&
                  Tick(f, SOTN_BUTTON_RIGHT));
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
        SotnPlayerState p = {0};
        CHECK(SotnPlayer_GetState(f->p, &p) && f->p->mp == mp && f->p->equipment[0] == 28 &&
              !f->consumed);
        if (k == 0)
            CHECK(f->factories == 1 && f->item == 176 && f->packed == 0x1038U &&
                  f->roll_count == 1 && p.throwing.used && p.throwing.keep &&
                  p.throwing.item == 176 && p.throwing_grunt == 0x6f2 && !p.hand_used &&
                  // Case129 sets the throw animation only from Player_Stand (or crouch).
                  (f->p->step == SOTN_PLAYER_WALK ? f->p->animation.id != 0x5d
                                                  : f->p->animation.id == 0x5d));
        else if (k < 3)
            CHECK(f->factories == 2 && !f->item && f->packed == 0x2aU && f->roll_count == 1 &&
                  p.hand_used && !p.throwing.used && f->p->attack_mode == 0x8001 &&
                  f->p->substep == 0x40 && f->p->animation.id == 0x24 &&
                  f->p->attack_lock == f->dra[0x500 + 21]);
        else
            CHECK(f->factories == 1 && f->item == 28 && f->packed == 0x1030U &&
                  f->roll_count == 1 && p.hand_used && f->p->attack_mode == 0x800d &&
                  f->p->substep == 0x4c && f->p->animation.id == 0x92 && f->p->attack_lock == 13);
    }
    CHECK(!SotnPlayer_SetSpecialCross(f->p, 49, 0));
    return true;
}
// 6E42C.c case29: Muramasa runs GetEquipProperties (one critical rand); at attack11 or more
// it swings normally, below (including poison's halving) it takes Red Rust's rand()&7 roll.
// 692E8.c func_8010A3F0 holds timer11 at0x20 and counts updates with absorbed blood.
static bool Muramasa(Fixture *f)
{
    const struct
    {
        Uint16 attack, first, second, rolls;
        bool poison, fails;
    } cases[] = {{8, 3, 5, 2, false, false},
                 {1, 3, 8, 2, false, true},
                 {1, 3, 9, 3, false, false},
                 {8, 3, 0x7ff8, 2, true, true}};
    for (Uint16 k = 0; k < SDL_arraysize(cases); ++k)
    {
        CHECK(Create(f, 141, cases[k].first, cases[k].second, true));
        Put16(f->dra + 0x500 + 141 * 52 + 8, cases[k].attack);
        SotnCharacterState progress = {0};
        CHECK(SotnCharacter_Get(f->character, &progress, NULL) &&
              SotnCharacter_Restore(f->character, &progress));
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        if (cases[k].poison)
            f->p->poison_ticks = 100;
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
        CHECK(f->factories == 1 && f->item == 141 && f->roll_count == cases[k].rolls &&
              f->p->curse_failure == cases[k].fails);
    }
    // Two-hand Muramasa holds the Dark Metamorphosis timer and reports blood updates; the
    // contact counter clears every living update.
    CHECK(Create(f, 141, 1, 2, true) && Tick(f, 0) && f->p->dark_ticks >= 0x1f &&
          !f->p->muramasa_blood);
    CHECK(SotnPlayer_QueueBloodHealing(f->p, 8, 2) && Tick(f, 0) && f->p->muramasa_blood &&
          !f->p->healing.contacts && Tick(f, 0) && !f->p->muramasa_blood);
    SotnPlayerState state = {0};
    CHECK(SotnPlayer_QueueBloodHealing(f->p, 0, 1) && Tick(f, 0) &&
          SotnPlayer_GetState(f->p, &state) && state.muramasa_blood);
    CHECK(Create(f, 26, 1, 2, true) && Tick(f, 0) && !f->p->dark_ticks &&
          SotnPlayer_QueueBloodHealing(f->p, 0, 1) && Tick(f, 0) && !f->p->muramasa_blood &&
          !f->p->healing.contacts);
    return true;
}
// 6E42C.c Combat knife: a lingering knife in slot16 selects special175 without any command;
// a lingering special selects the ordinary knife even after back-forward; kinds9/10 skip grunt.
static bool CombatKnife(Fixture *f)
{
    const Uint16 slots[] = {20, 175, 0}, expected[] = {175, 20, 175};
    for (Uint16 k = 0; k < SDL_arraysize(slots); ++k)
    {
        CHECK(Create(f, 20, 1, 2, true));
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(Tick(f, 0));
        if (k)
            CHECK(Tick(f, SOTN_BUTTON_LEFT) && Tick(f, SOTN_BUTTON_RIGHT) && Tick(f, 0) &&
                  f->p->back_forward.phase == 255);
        CHECK(SotnPlayer_SetSlot16Item(f->p, slots[k]) && Tick(f, SOTN_BUTTON_LEFT_HAND));
        CHECK(f->factories == 1 && f->item == expected[k] && !f->roll_count &&
              f->p->attack_mode == (expected[k] == 175 ? 0x800b : 0x800a));
    }
    CHECK(!SotnPlayer_SetSlot16Item(f->p, 217));
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
         Roll(f, 7, false) && BackForward(f) && KindFourSpecial(f) && FistCombo(f) &&
         QuarterCircle(f) && HolySword(f) && Brand(f) && Warp(f) && Lunge(f) && Punches(f) &&
         Weapon51Specials(f) && MoonRod(f) && Runesword(f, 143) && Runesword(f, 119) &&
         Runesword(f, 133) && Runesword(f, 163) && Runesword(f, 164) && Runesword(f, 165) &&
         Muramasa(f) && CombatKnife(f) && AirborneKindZero(f) && Shotel(f) && SwordOfDawn(f) &&
         Combo(f, 5, 203, 10) && Combo(f, 8, 206, 20) && Combo(f, 11, 209, 40);
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
